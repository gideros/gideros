#include "prpath.h"
#include "kvec.h"

#define FT_THROW(error) 1
#define FT_TRACE5(varformat)

  typedef int
  (*FT_Outline_CloseFunc)( void*  user );

  typedef struct  FT_Outline_Funcs_Ex_
  {
    FT_Outline_MoveToFunc   move_to;
    FT_Outline_LineToFunc   line_to;
    FT_Outline_ConicToFunc  conic_to;
    FT_Outline_CubicToFunc  cubic_to;
    FT_Outline_CloseFunc    close;

    int                     shift;
    FT_Pos                  delta;

  } FT_Outline_Funcs_Ex;

  static FT_Error
  FT_Outline_Decompose_Ex( FT_Outline*                 outline,
                           const FT_Outline_Funcs_Ex*  func_interface,
                           void*                       user )
  {
#undef SCALED
#define SCALED( x )  ( ( (x) << shift ) - delta )

    FT_Vector   v_last;
    FT_Vector   v_control;
    FT_Vector   v_start;

    FT_Vector*  point;
    FT_Vector*  limit;
    char*       tags;

    FT_Error    error;

    FT_Int   n;         /* index of contour in outline     */
    FT_UInt  first;     /* index of first point in contour */
    FT_Int   tag;       /* current point's state           */

    FT_Int   shift;
    FT_Pos   delta;


    if ( !outline || !func_interface )
      return FT_THROW( Invalid_Argument );

    shift = func_interface->shift;
    delta = func_interface->delta;
    first = 0;

    for ( n = 0; n < outline->n_contours; n++ )
    {
      FT_Int  last;  /* index of last point in contour */


      FT_TRACE5(( "FT_Outline_Decompose: Outline %d\n", n ));

      last = outline->contours[n];
      if ( last < 0 )
        goto Invalid_Outline;
      limit = outline->points + last;

      v_start   = outline->points[first];
      v_start.x = SCALED( v_start.x );
      v_start.y = SCALED( v_start.y );

      v_last   = outline->points[last];
      v_last.x = SCALED( v_last.x );
      v_last.y = SCALED( v_last.y );

      v_control = v_start;

      point = outline->points + first;
      tags  = outline->tags   + first;
      tag   = FT_CURVE_TAG( tags[0] );

      /* A contour cannot start with a cubic control point! */
      if ( tag == FT_CURVE_TAG_CUBIC )
        goto Invalid_Outline;

      /* check first point to determine origin */
      if ( tag == FT_CURVE_TAG_CONIC )
      {
        /* first point is conic control.  Yes, this happens. */
        if ( FT_CURVE_TAG( outline->tags[last] ) == FT_CURVE_TAG_ON )
        {
          /* start at last point if it is on the curve */
          v_start = v_last;
          limit--;
        }
        else
        {
          /* if both first and last points are conic,         */
          /* start at their middle and record its position    */
          /* for closure                                      */
          v_start.x = ( v_start.x + v_last.x ) / 2;
          v_start.y = ( v_start.y + v_last.y ) / 2;

       /* v_last = v_start; */
        }
        point--;
        tags--;
      }

      FT_TRACE5(( "  move to (%.2f, %.2f)\n",
                  v_start.x / 64.0, v_start.y / 64.0 ));
      error = func_interface->move_to( &v_start, user );
      if ( error )
        goto Exit;

      while ( point < limit )
      {
        point++;
        tags++;

        tag = FT_CURVE_TAG( tags[0] );
        switch ( tag )
        {
        case FT_CURVE_TAG_ON:  /* emit a single line_to */
          {
            FT_Vector  vec;


            vec.x = SCALED( point->x );
            vec.y = SCALED( point->y );

            FT_TRACE5(( "  line to (%.2f, %.2f)\n",
                        vec.x / 64.0, vec.y / 64.0 ));
            error = func_interface->line_to( &vec, user );
            if ( error )
              goto Exit;
            continue;
          }

        case FT_CURVE_TAG_CONIC:  /* consume conic arcs */
          v_control.x = SCALED( point->x );
          v_control.y = SCALED( point->y );

        Do_Conic:
          if ( point < limit )
          {
            FT_Vector  vec;
            FT_Vector  v_middle;


            point++;
            tags++;
            tag = FT_CURVE_TAG( tags[0] );

            vec.x = SCALED( point->x );
            vec.y = SCALED( point->y );

            if ( tag == FT_CURVE_TAG_ON )
            {
              FT_TRACE5(( "  conic to (%.2f, %.2f)"
                          " with control (%.2f, %.2f)\n",
                          vec.x / 64.0, vec.y / 64.0,
                          v_control.x / 64.0, v_control.y / 64.0 ));
              error = func_interface->conic_to( &v_control, &vec, user );
              if ( error )
                goto Exit;
              continue;
            }

            if ( tag != FT_CURVE_TAG_CONIC )
              goto Invalid_Outline;

            v_middle.x = ( v_control.x + vec.x ) / 2;
            v_middle.y = ( v_control.y + vec.y ) / 2;

            FT_TRACE5(( "  conic to (%.2f, %.2f)"
                        " with control (%.2f, %.2f)\n",
                        v_middle.x / 64.0, v_middle.y / 64.0,
                        v_control.x / 64.0, v_control.y / 64.0 ));
            error = func_interface->conic_to( &v_control, &v_middle, user );
            if ( error )
              goto Exit;

            v_control = vec;
            goto Do_Conic;
          }

          FT_TRACE5(( "  conic to (%.2f, %.2f)"
                      " with control (%.2f, %.2f)\n",
                      v_start.x / 64.0, v_start.y / 64.0,
                      v_control.x / 64.0, v_control.y / 64.0 ));
          error = func_interface->conic_to( &v_control, &v_start, user );
          goto Close;

        default:  /* FT_CURVE_TAG_CUBIC */
          {
            FT_Vector  vec1, vec2;


            if ( point + 1 > limit                             ||
                 FT_CURVE_TAG( tags[1] ) != FT_CURVE_TAG_CUBIC )
              goto Invalid_Outline;

            point += 2;
            tags  += 2;

            vec1.x = SCALED( point[-2].x );
            vec1.y = SCALED( point[-2].y );

            vec2.x = SCALED( point[-1].x );
            vec2.y = SCALED( point[-1].y );

            if ( point <= limit )
            {
              FT_Vector  vec;


              vec.x = SCALED( point->x );
              vec.y = SCALED( point->y );

              FT_TRACE5(( "  cubic to (%.2f, %.2f)"
                          " with controls (%.2f, %.2f) and (%.2f, %.2f)\n",
                          vec.x / 64.0, vec.y / 64.0,
                          vec1.x / 64.0, vec1.y / 64.0,
                          vec2.x / 64.0, vec2.y / 64.0 ));
              error = func_interface->cubic_to( &vec1, &vec2, &vec, user );
              if ( error )
                goto Exit;
              continue;
            }

            FT_TRACE5(( "  cubic to (%.2f, %.2f)"
                        " with controls (%.2f, %.2f) and (%.2f, %.2f)\n",
                        v_start.x / 64.0, v_start.y / 64.0,
                        vec1.x / 64.0, vec1.y / 64.0,
                        vec2.x / 64.0, vec2.y / 64.0 ));
            error = func_interface->cubic_to( &vec1, &vec2, &v_start, user );
            goto Close;
          }
        }
      }

      /* close the contour */
      FT_TRACE5(( "  close\n" ));
      error = func_interface->close( user );

    Close:
      if ( error )
        goto Exit;

      first = last + 1;
    }

    FT_TRACE5(( "FT_Outline_Decompose: Done\n", n ));
    return FT_Err_Ok;

  Exit:
    FT_TRACE5(( "FT_Outline_Decompose: Error %d\n", error ));
    return error;

  Invalid_Outline:
    return FT_THROW( Invalid_Outline );
  }

struct decompose_data
{
    kvec_t(unsigned char) commands;
    kvec_t(float) coords;
};

static int moveto(const FT_Vector *to, void *user)
{
    struct decompose_data *data = user;

    kv_push_back(data->commands, 'M');
    kv_push_back(data->coords, to->x / 64.f);
    kv_push_back(data->coords, -to->y / 64.f);

    return 0;
}
static int lineto(const FT_Vector *to, void *user)
{
    struct decompose_data *data = user;

    kv_push_back(data->commands, 'L');
    kv_push_back(data->coords, to->x / 64.f);
    kv_push_back(data->coords, -to->y / 64.f);

    return 0;
}

static int conicto(const FT_Vector *control, const FT_Vector *to, void *user)
{
    struct decompose_data *data = user;

    kv_push_back(data->commands, 'Q');
    kv_push_back(data->coords, control->x / 64.f);
    kv_push_back(data->coords, -control->y / 64.f);
    kv_push_back(data->coords, to->x / 64.f);
    kv_push_back(data->coords, -to->y / 64.f);

    return 0;
}

static int cubicto(const FT_Vector *control1, const FT_Vector *control2, const FT_Vector *to, void *user)
{
    struct decompose_data *data = user;

    kv_push_back(data->commands, 'C');
    kv_push_back(data->coords, control1->x / 64.f);
    kv_push_back(data->coords, -control1->y / 64.f);
    kv_push_back(data->coords, control2->x / 64.f);
    kv_push_back(data->coords, -control2->y / 64.f);
    kv_push_back(data->coords, to->x / 64.f);
    kv_push_back(data->coords, -to->y / 64.f);

    return 0;
}

static int close(void *user)
{
    struct decompose_data *data = user;

    kv_push_back(data->commands, 'Z');

    return 0;
}

struct PrPath *prParseFtGlyph(FT_Outline *outline)
{
    struct decompose_data data;
    kv_init(data.commands);
    kv_init(data.coords);

    const FT_Outline_Funcs_Ex funcs = {moveto, lineto, conicto, cubicto, close, 0, 0};
    if (FT_Outline_Decompose_Ex(outline, &funcs, &data))
        return NULL;

    struct PrPath *p = malloc(sizeof(struct PrPath));

    p->numCommands = kv_size(data.commands);
    p->commands = kv_data(data.commands);
    p->numCoords = kv_size(data.coords);
    p->coords = kv_data(data.coords);

    return p;
}

