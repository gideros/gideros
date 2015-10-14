
GLuint glGenPathsAC(GLsizei range)
{
    return gen_paths(range);
}

void glDeletePathsAC(GLuint path, GLsizei range)
{
    delete_paths(path, range);
}

GLboolean glIsPathAC(GLuint path)
{
    return kh_get(path, paths, path) != kh_end(paths);
}

void glPathCommandsAC(GLuint path, GLsizei numCommands, const GLubyte *commands, GLsizei numCoords, GLenum coordType, const GLvoid *coords)
{
    float *coords2;
    if (coordType != GL_BYTE &&
        coordType != GL_UNSIGNED_BYTE &&
        coordType != GL_SHORT &&
        coordType != GL_UNSIGNED_SHORT &&
        coordType != GL_FLOAT)
    {
        // TODO: set error
        return;
    }

    coords2 = (float*)malloc(numCoords * sizeof(float));

    if (coordType == GL_BYTE)
    {
        GLbyte *coords3 = (GLbyte*)coords;
        int i;
        for (i = 0; i < numCoords; ++i)
            coords2[i] = coords3[i];
    }
    else if (coordType == GL_UNSIGNED_BYTE)
    {
        GLubyte *coords3 = (GLubyte*)coords;
        int i;
        for (i = 0; i < numCoords; ++i)
            coords2[i] = coords3[i];
    }
    else if (coordType == GL_SHORT)
    {
        GLshort *coords3 = (GLshort*)coords;
        int i;
        for (i = 0; i < numCoords; ++i)
            coords2[i] = coords3[i];
    }
    else if (coordType == GL_UNSIGNED_SHORT)
    {
        GLushort *coords3 = (GLushort*)coords;
        int i;
        for (i = 0; i < numCoords; ++i)
            coords2[i] = coords3[i];
    }
    else if (coordType == GL_FLOAT)
    {
        GLfloat *coords3 = (GLfloat*)coords;
        int i;
        for (i = 0; i < numCoords; ++i)
            coords2[i] = coords3[i];
    }

    path_commands(path, numCommands, commands, numCoords, coords2);

    free(coords2);
}

static void stencil_stroke_path(GLuint path, GLint reference, GLuint mask, const float matrix[16])
{
    khiter_t iter = kh_get(path, paths, path);
    if (iter == kh_end(paths))
        return;

    struct path *p = kh_val(paths, iter);

    if (p->is_stroke_dirty)
    {
        create_stroke_geometry(p);
        p->is_stroke_dirty = 0;
    }

    // save
    GLboolean cmask[4], dmask;
    GLint smask;
    glGetBooleanv(GL_COLOR_WRITEMASK, cmask);
    glGetBooleanv(GL_DEPTH_WRITEMASK, &dmask);
    glGetIntegerv(GL_STENCIL_WRITEMASK, &smask);

    GLint sfunc, sref, svmask;
    glGetIntegerv(GL_STENCIL_FUNC, &sfunc);
    glGetIntegerv(GL_STENCIL_REF, &sref);
    glGetIntegerv(GL_STENCIL_VALUE_MASK, &svmask);

    GLenum fail, zfail, zpass;
    glGetIntegerv(GL_STENCIL_FAIL, &fail);
    glGetIntegerv(GL_STENCIL_PASS_DEPTH_FAIL, &zfail);
    glGetIntegerv(GL_STENCIL_PASS_DEPTH_PASS, &zpass);

    GLint currentProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);

    GLint data0Enabled, data1Enabled, data2Enabled;
    glGetVertexAttribiv(DATA0_POS, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &data0Enabled);
    glGetVertexAttribiv(DATA1_POS, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &data1Enabled);
    glGetVertexAttribiv(DATA2_POS, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &data2Enabled);

    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDepthMask(GL_FALSE);
    glStencilMask(mask);
    glStencilFunc(GL_ALWAYS, reference, ~0);    // TODO: this should be configured with glPathStencilFuncAC
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    if (p->stroke_geoms[0].count > 0)
    {
        glUseProgram(program0);
        glUniformMatrix4fv(matrix0, 1, GL_FALSE, matrix);
#ifdef OPENGL_ES
        glUniformMatrix4fv(mvp0, 1, GL_FALSE, g_mvp);
#endif

        glEnableVertexAttribArray(DATA0_POS);

        glBindBuffer(GL_ARRAY_BUFFER, p->stroke_geoms[0].vertex_buffer);
        glVertexAttribPointer(DATA0_POS, 2, GL_FLOAT, GL_FALSE, 8, BUFFER_OFFSET(0));
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, p->stroke_geoms[0].index_buffer);
        glDrawElements(GL_TRIANGLES, p->stroke_geoms[0].count, GL_UNSIGNED_SHORT, BUFFER_OFFSET(0));
    }

    if (p->stroke_geoms[1].count > 0)
    {
        glUseProgram(program2);
        glUniformMatrix4fv(matrix2, 1, GL_FALSE, matrix);
#ifdef OPENGL_ES
        glUniformMatrix4fv(mvp2, 1, GL_FALSE, g_mvp);
#endif

        glEnableVertexAttribArray(DATA0_POS);
        glEnableVertexAttribArray(DATA1_POS);
        glEnableVertexAttribArray(DATA2_POS);

        glBindBuffer(GL_ARRAY_BUFFER, p->stroke_geoms[1].vertex_buffer);
        glVertexAttribPointer(DATA0_POS, 4, GL_FLOAT, GL_FALSE, 48, BUFFER_OFFSET(0 * 4));
        glVertexAttribPointer(DATA1_POS, 4, GL_FLOAT, GL_FALSE, 48, BUFFER_OFFSET(4 * 4));
        glVertexAttribPointer(DATA2_POS, 4, GL_FLOAT, GL_FALSE, 48, BUFFER_OFFSET(8 * 4));
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, p->stroke_geoms[1].index_buffer);
        glDrawElements(GL_TRIANGLES, p->stroke_geoms[1].count, GL_UNSIGNED_SHORT, BUFFER_OFFSET(0));
    }

    // restore
    if (!data0Enabled)
        glDisableVertexAttribArray(DATA0_POS);
    if (!data1Enabled)
        glDisableVertexAttribArray(DATA1_POS);
    if (!data2Enabled)
        glDisableVertexAttribArray(DATA2_POS);

    glUseProgram(currentProgram);

    glColorMask(cmask[0], cmask[1], cmask[2], cmask[3]);
    glDepthMask(dmask);
    glStencilMask(smask);
    glStencilFunc(sfunc, sref, svmask);
    glStencilOp(fail, zfail, zpass);

    /* TODO: save/restore */
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void glStencilStrokePathAC(GLuint path, GLint reference, GLuint mask)
{
    stencil_stroke_path(path, reference, mask, identity_matrix);
}

static void stencil_fill_path(GLuint path, GLenum fill_mode, GLuint mask, const GLfloat matrix[16])
{
    struct path *p = NULL;

    khiter_t iter = kh_get(path, paths, path);
    if (iter == kh_end(paths))
    {
        // TODO: check if we should set error here
        return;
    }

    GLenum front, back;
    switch (fill_mode)
    {
    case GL_COUNT_UP_AC:
        front = GL_INCR_WRAP;
        back = GL_DECR_WRAP;
        break;
    case GL_COUNT_DOWN_AC:
        front = GL_DECR_WRAP;
        back = GL_INCR_WRAP;
        break;
    case GL_INVERT:
        front = GL_INVERT;
        back = GL_INVERT;
        break;
    default:
        // TODO: check if we should set error here
        return;
    }

    p = kh_val(paths, iter);

    if (p->is_fill_dirty)
    {
        create_fill_geometry(p);
        p->is_fill_dirty = 0;
    }

    // save
    GLboolean cmask[4], dmask;
    GLint smask;
    glGetBooleanv(GL_COLOR_WRITEMASK, cmask);
    glGetBooleanv(GL_DEPTH_WRITEMASK, &dmask);
    glGetIntegerv(GL_STENCIL_WRITEMASK, &smask);

    GLint sfunc, sref, svmask;
    glGetIntegerv(GL_STENCIL_FUNC, &sfunc);
    glGetIntegerv(GL_STENCIL_REF, &sref);
    glGetIntegerv(GL_STENCIL_VALUE_MASK, &svmask);

    GLenum fail, zfail, zpass;
    glGetIntegerv(GL_STENCIL_FAIL, &fail);
    glGetIntegerv(GL_STENCIL_PASS_DEPTH_FAIL, &zfail);
    glGetIntegerv(GL_STENCIL_PASS_DEPTH_PASS, &zpass);

    GLint currentProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);

    GLint data0Enabled;
    glGetVertexAttribiv(DATA0_POS, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &data0Enabled);

    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDepthMask(GL_FALSE);
    glStencilMask(mask);
    glStencilFunc(GL_ALWAYS, 0, ~0);    // TODO: this should be configured with glPathStencilFuncAC

    glEnableVertexAttribArray(DATA0_POS);

    glBindBuffer(GL_ARRAY_BUFFER, p->fill_vertex_buffer);
    glVertexAttribPointer(DATA0_POS, 4, GL_FLOAT, GL_FALSE, 16, BUFFER_OFFSET(0));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, p->fill_index_buffer);

    glUseProgram(program1);
    glUniformMatrix4fv(matrix1, 1, GL_FALSE, matrix);
#ifdef OPENGL_ES
    glUniformMatrix4fv(mvp1, 1, GL_FALSE, g_mvp);
#endif

    if (p->fill_counts[0] > 0)
    {
        glStencilOp(GL_KEEP, GL_KEEP, front);
        glDrawElements(GL_TRIANGLES, p->fill_counts[0], GL_UNSIGNED_SHORT, BUFFER_OFFSET(p->fill_starts[0] * 2));
    }

    if (p->fill_counts[1] > 0)
    {
        glStencilOp(GL_KEEP, GL_KEEP, back);
        glDrawElements(GL_TRIANGLES, p->fill_counts[1], GL_UNSIGNED_SHORT, BUFFER_OFFSET(p->fill_starts[1] * 2));
    }

    // restore
    if (!data0Enabled)
        glDisableVertexAttribArray(DATA0_POS);

    glUseProgram(currentProgram);

    glColorMask(cmask[0], cmask[1], cmask[2], cmask[3]);
    glDepthMask(dmask);
    glStencilMask(smask);
    glStencilFunc(sfunc, sref, svmask);
    glStencilOp(fail, zfail, zpass);

    /* TODO: save/restore */
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void glStencilFillPathAC(GLuint path, GLenum fillMode, GLuint mask)
{
    stencil_fill_path(path, fillMode, mask, identity_matrix);
}

void glPathParameterfAC(GLuint path, GLenum pname, GLfloat value)
{
    struct path *p = NULL;

    khiter_t iter = kh_get(path, paths, path);
    if (iter == kh_end(paths))
    {
        // TODO: check if we should set error here
        return;
    }

    p = kh_val(paths, iter);

    switch (pname)
    {
    case GL_PATH_STROKE_WIDTH_AC:
        p->stroke_width = value;
        p->is_stroke_dirty = 1;
        break;
    case GL_PATH_MITER_LIMIT_AC:
        p->miter_limit = value;
        p->is_stroke_dirty = 1;
        break;
    default:
        // TODO: check if we should set error here
        break;
    }


}

void glPathParameteriAC(GLuint path, GLenum pname, GLint value)
{
    khiter_t iter;
    struct path *p;

    iter = kh_get(path, paths, path);
    if (iter == kh_end(paths))
    {
        // TODO: check if we should set error here
        return;
    }

    p = kh_val(paths, iter);

    switch (pname)
    {
    case GL_PATH_JOIN_STYLE_AC:
        if (value != GL_MITER_REVERT_AC && value != GL_MITER_TRUNCATE_AC && value != GL_BEVEL_AC && value != GL_ROUND_AC && value != GL_NONE)
        {
            /* TODO: check if we should set error here */
            return;
        }
        p->join_style = value;
        p->is_stroke_dirty = 1;
        break;
    case GL_PATH_INITIAL_END_CAP_AC:
        if (value != GL_FLAT && value != GL_SQUARE_AC && value != GL_ROUND_AC && value != GL_TRIANGULAR_AC)
        {
            /* TODO: check if we should set error here */
            return;
        }
        p->initial_end_cap = value;
        p->is_stroke_dirty = 1;
        break;
        break;
    case GL_PATH_TERMINAL_END_CAP_AC:
        if (value != GL_FLAT && value != GL_SQUARE_AC && value != GL_ROUND_AC && value != GL_TRIANGULAR_AC)
        {
            /* TODO: check if we should set error here */
            return;
        }
        p->terminal_end_cap = value;
        p->is_stroke_dirty = 1;
        break;
        break;
    default:
        /* TODO: check if we should set error here */
        break;
    }
}


static int get_path_name(int pathNameType, const void **paths, unsigned int pathBase, unsigned int *pathName)
{
    typedef signed char byte;
    typedef unsigned char ubyte;
    typedef unsigned short ushort;
    typedef unsigned int uint;

    switch (pathNameType)
    {
    case GL_BYTE:
    {
        const byte *p = *paths;
        *pathName = pathBase + p[0];
        *paths = p + 1;
        return 1;
    }
    case GL_UNSIGNED_BYTE:
    {
        const ubyte *p = *paths;
        *pathName = pathBase + p[0];
        *paths = p + 1;
        return 1;
    }
    case GL_SHORT:
    {
        const short *p = *paths;
        *pathName = pathBase + p[0];
        *paths = p + 1;
        return 1;
    }
    case GL_UNSIGNED_SHORT:
    {
        const ushort *p = *paths;
        *pathName = pathBase + p[0];
        *paths = p + 1;
        return 1;
    }
    case GL_INT:
    {
        const int *p = *paths;
        *pathName = pathBase + p[0];
        *paths = p + 1;
        return 1;
    }
    case GL_UNSIGNED_INT:
    {
        const uint *p = *paths;
        *pathName = pathBase + p[0];
        *paths = p + 1;
        return 1;
    }
    case GL_FLOAT:
    {
        const float *p = *paths;
        *pathName = pathBase + p[0];
        *paths = p + 1;
        return 1;
    }
    case GL_2_BYTES:
    {
        const ubyte *p = *paths;
        *pathName = pathBase + (p[0] << 8 | p[1]);
        *paths = p + 2;
        return 1;
    }
    case GL_3_BYTES:
    {
        const ubyte *p = *paths;
        *pathName = pathBase + (p[0] << 16 | p[1] << 8 | p[0]);
        *paths = p + 3;
        return 1;
    }
    case GL_4_BYTES:
    {
        const ubyte *p = *paths;
        *pathName = pathBase + (p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3]);
        *paths = p + 4;
        return 1;
    }
    case GL_UTF8_AC:
    {
        const ubyte *p = *paths;
        ubyte c0 = p[0];
        if ((c0 & 0x80) == 0x00)
        {
            /* Zero continuation (0 to 127) */
            *pathName = pathBase + c0;
            p += 1;
        }
        else
        {
            ubyte c1 = p[1];
            if ((c1 & 0xC0) != 0x80)
            {
                /* Stop processing the UTF byte sequence early. */
                return 0;
            }
            if ((c0 & 0xE0) == 0xC0)
            {
                /* One contination (128 to 2047) */
                *pathName = pathBase + ((c1 & 0x3F) | (c0 & 0x1F) << 6);
                if (*pathName < 128)
                {
                    return 0;
                }
                p += 2;
            }
            else
            {
                ubyte c2 = p[2];
                if ((c2 & 0xC0) != 0x80)
                {
                    /* Stop processing the UTF byte sequence early. */
                    return 0;
                }
                if ((c0 & 0xF0) == 0xE0)
                {
                    /* Two continuation (2048 to 55295 and 57344 to 65535) */
                    *pathName = pathBase + ((c2 & 0x3F) | (c1 & 0x3F) << 6 | (c0 & 0xF) << 12);
                    if ((*pathName >= 55296) && (*pathName <= 57343))
                    {
                        /* Stop processing the UTF byte sequence early. */
                        return 0;
                    }
                    if (*pathName < 2048)
                    {
                        return 0;
                    }
                    p += 3;
                }
                else
                {
                    ubyte c3 = p[3];
                    if ((c3 & 0xC0) != 0x80)
                    {
                        /* Stop processing the UTF byte sequence early. */
                        return 0;
                    }
                    if ((c0 & 0xF8) == 0xF0)
                    {
                        /* Three continuation (65536 to 1114111) */
                        *pathName = pathBase + ((c3 & 0x3F) | (c2 & 0x3F) << 6 | (c1 & 0x3F) << 12 | (c0 & 0x7) << 18);
                        if (*pathName < 65536 && *pathName > 1114111)
                        {
                            return 0;
                        }
                        p += 4;
                    }
                    else
                    {
                        /* Skip invalid or restricted encodings. */
                        /* Stop processing the UTF byte sequence early. */
                        return 0;
                    }
                }
            }
        }
        *paths = p;
        return 1;
    }
    case GL_UTF16_AC:
    {
        const ushort *p = *paths;

        ushort s0 = p[0];
        if ((s0 < 0xDB00) || (s0 > 0xDFFF))
        {
            *pathName = pathBase + s0;
            p += 1;
        }
        else
        {
            if ((s0 >= 0xDB00) && (s0 <= 0xDBFF))
            {
                ushort s1 = p[1];
                if ((s1 >= 0xDC00) && (s1 <= 0xDFFF))
                {
                    *pathName = pathBase + (((s0 & 0x3FF) << 10 | (s1 & 0x3FF)) + 0x10000);
                    p += 2;
                }
                else
                {
                    /* Stop processing the UTF byte sequence early. */
                    return 0;
                }
            }
            else
            {
                return 0;
            }
        }
        *paths = p;
        return 1;
    }
    default:  /* TODO: generate INVALID_ENUM */
        return 0;
    }
}

static const float *apply_transform_type(int transform_type, const float *v, float m[16])
{
    switch (transform_type)
    {
    case GL_NONE:
        m[0] = 1; m[4] = 0; m[8] = 0;  m[12] = 0;
        m[1] = 0; m[5] = 1; m[9] = 0;  m[13] = 0;
        m[2] = 0; m[6] = 0; m[10] = 1; m[14] = 0;
        m[3] = 0; m[7] = 0; m[11] = 0; m[15] = 1;
        break;
    case GL_TRANSLATE_X_AC:
        m[0] = 1; m[4] = 0; m[8] = 0;  m[12] = v[0];
        m[1] = 0; m[5] = 1; m[9] = 0;  m[13] = 0;
        m[2] = 0; m[6] = 0; m[10] = 1; m[14] = 0;
        m[3] = 0; m[7] = 0; m[11] = 0; m[15] = 1;
        v += 1;
        break;
    case GL_TRANSLATE_Y_AC:
        m[0] = 1; m[4] = 0; m[8] = 0;  m[12] = 0;
        m[1] = 0; m[5] = 1; m[9] = 0;  m[13] = v[0];
        m[2] = 0; m[6] = 0; m[10] = 1; m[14] = 0;
        m[3] = 0; m[7] = 0; m[11] = 0; m[15] = 1;
        v += 1;
        break;
    case GL_TRANSLATE_2D_AC:
        m[0] = 1; m[4] = 0; m[8] = 0;  m[12] = v[0];
        m[1] = 0; m[5] = 1; m[9] = 0;  m[13] = v[1];
        m[2] = 0; m[6] = 0; m[10] = 1; m[14] = 0;
        m[3] = 0; m[7] = 0; m[11] = 0; m[15] = 1;
        v += 2;
        break;
    case GL_TRANSLATE_3D_AC:
        m[0] = 1; m[4] = 0; m[8] = 0;  m[12] = v[0];
        m[1] = 0; m[5] = 1; m[9] = 0;  m[13] = v[1];
        m[2] = 0; m[6] = 0; m[10] = 1; m[14] = v[2];
        m[3] = 0; m[7] = 0; m[11] = 0; m[15] = 1;
        v += 3;
        break;
    case GL_AFFINE_2D_AC:
        m[0] = v[0]; m[4] = v[2]; m[8] = 0;  m[12] = v[4];
        m[1] = v[1]; m[5] = v[3]; m[9] = 0;  m[13] = v[5];
        m[2] = 0;    m[6] = 0;    m[10] = 1; m[14] = 0;
        m[3] = 0;    m[7] = 0;    m[11] = 0; m[15] = 1;
        v += 6;
        break;
    case GL_TRANSPOSE_AFFINE_2D_AC:
        m[0] = v[0]; m[4] = v[1]; m[8] = 0;  m[12] = v[2];
        m[1] = v[3]; m[5] = v[4]; m[9] = 0;  m[13] = v[5];
        m[2] = 0;    m[6] = 0;    m[10] = 1; m[14] = 0;
        m[3] = 0;    m[7] = 0;    m[11] = 0; m[15] = 1;
        v += 6;
        break;
    case GL_AFFINE_3D_AC:
        m[0] = v[0]; m[4] = v[3]; m[8] = v[6];  m[12] = v[9];
        m[1] = v[1]; m[5] = v[4]; m[9] = v[7];  m[13] = v[10];
        m[2] = v[2]; m[6] = v[5]; m[10] = v[8]; m[14] = v[11];
        m[3] = 0;    m[7] = 0;    m[11] = 1;    m[15] = 0;
        v += 12;
        break;
    case GL_TRANSPOSE_AFFINE_3D_AC:
        m[0] = v[0]; m[4] = v[1]; m[8] = v[2];   m[12] = v[3];
        m[1] = v[4]; m[5] = v[5]; m[9] = v[6];   m[13] = v[7];
        m[2] = v[8]; m[6] = v[9]; m[10] = v[10]; m[14] = v[11];
        m[3] = 0;    m[7] = 0;    m[11] = 1;     m[15] = 0;
        v += 12;
        break;
    default:  /* TODO: generate INVALID_ENUM */
        break;
    }
    return v;
}

void glStencilFillPathInstancedAC(GLsizei numPaths, GLenum pathNameType, const void* paths, GLuint pathBase, GLenum fillMode, GLuint mask, GLenum transformType, const GLfloat *transformValues)
{
    int i;
    const float *v;
    
    v = transformValues;
    for (i = 0; i < numPaths; ++i)
    {
        float m[16];
        unsigned int pathName;

        v = apply_transform_type(transformType, v, m);
        if (v == NULL)
            return;
        
        if (!get_path_name(pathNameType, &paths, pathBase, &pathName))
            return;

        if (glIsPathAC(pathName))
            stencil_fill_path(pathName, fillMode, mask, m);
    }
}

void glStencilStrokePathInstancedAC(GLsizei numPaths, GLenum pathNameType, const void* paths, GLuint pathBase, GLint reference, GLuint mask, GLenum transformType, const GLfloat *transformValues)
{
    int i;
    const float *v;

    v = transformValues;
    for (i = 0; i < numPaths; ++i)
    {
        float m[16];
        unsigned int pathName;

        v = apply_transform_type(transformType, v, m);
        if (v == NULL)
            return;

        if (!get_path_name(pathNameType, &paths, pathBase, &pathName))
            return;

        if (glIsPathAC(pathName))
            stencil_stroke_path(pathName, reference, mask, m);
    }
}

void glGetPathBoundingBoxInstancedAC(GLenum boundingBoxType, GLsizei numPaths, GLenum pathNameType, const void* paths, GLuint pathBase, GLenum transformType, const GLfloat *transformValues, GLfloat *result)
{
    int i;
    int hasBounds = 0;
    float boundsUnion[4], bounds[4];

    const float *v = transformValues;
    for (i = 0; i < numPaths; i++)
    {
        unsigned int pathName;
        if (!get_path_name(pathNameType, &paths, pathBase, &pathName))
            return;

        if (glIsPathAC(pathName))
        {
            glGetPathParameterfvAC(pathName, boundingBoxType, bounds);
            switch (transformType)
            {
            case GL_NONE:
                break;
            case GL_TRANSLATE_X_AC:
                bounds[0] += v[0];
                bounds[2] += v[0];
                v += 1;
                break;
            case GL_TRANSLATE_Y_AC:
                bounds[1] += v[0];
                bounds[3] += v[0];
                v += 1;
                break;
            case GL_TRANSLATE_2D_AC:
                bounds[0] += v[0];
                bounds[1] += v[1];
                bounds[2] += v[0];
                bounds[3] += v[1];
                v += 2;
                break;
            case GL_TRANSLATE_3D_AC: /* ignores v[2] */
                bounds[0] += v[0];
                bounds[1] += v[1];
                bounds[2] += v[0];
                bounds[3] += v[1];
                v += 3;
                break;
            case GL_AFFINE_2D_AC:
                bounds[0] = bounds[0] * v[0] + bounds[0] * v[2] + v[4];
                bounds[1] = bounds[1] * v[1] + bounds[1] * v[3] + v[5];
                bounds[2] = bounds[2] * v[0] + bounds[2] * v[2] + v[4];
                bounds[3] = bounds[3] * v[1] + bounds[3] * v[3] + v[5];
                v += 6;
                break;
            case GL_TRANSPOSE_AFFINE_2D_AC:
                bounds[0] = bounds[0] * v[0] + bounds[0] * v[1] + v[2];
                bounds[1] = bounds[1] * v[3] + bounds[1] * v[4] + v[5];
                bounds[2] = bounds[2] * v[0] + bounds[2] * v[1] + v[2];
                bounds[3] = bounds[3] * v[3] + bounds[3] * v[4] + v[5];
                v += 6;
                break;
            case GL_AFFINE_3D_AC:  /* ignores v[2], v[5], v[6..8], v[11] */
                bounds[0] = bounds[0] * v[0] + bounds[0] * v[3] + v[9];
                bounds[1] = bounds[1] * v[1] + bounds[1] * v[4] + v[10];
                bounds[2] = bounds[2] * v[0] + bounds[2] * v[3] + v[9];
                bounds[3] = bounds[3] * v[1] + bounds[3] * v[4] + v[10];
                v += 12;
                break;
            case GL_TRANSPOSE_AFFINE_3D_AC:  /* ignores v[2], v[6], v[8..11] */
                bounds[0] = bounds[0] * v[0] + bounds[0] * v[1] + v[3];
                bounds[1] = bounds[1] * v[4] + bounds[1] * v[5] + v[7];
                bounds[2] = bounds[2] * v[0] + bounds[2] * v[1] + v[3];
                bounds[3] = bounds[3] * v[4] + bounds[3] * v[5] + v[7];
                v += 12;
                break;
            default:  /* TODO: generate INVALID_ENUM */
                break;
            }

            if (bounds[0] > bounds[2])
            {
                float t = bounds[2];
                bounds[2] = bounds[0];
                bounds[0] = t;
            }

            if (bounds[1] > bounds[3])
            {
                float t = bounds[3];
                bounds[3] = bounds[1];
                bounds[1] = t;
            }

            if (hasBounds)
            {
                boundsUnion[0] = MIN(boundsUnion[0], bounds[0]);
                boundsUnion[1] = MIN(boundsUnion[1], bounds[1]);
                boundsUnion[2] = MAX(boundsUnion[2], bounds[2]);
                boundsUnion[3] = MAX(boundsUnion[3], bounds[3]);
            }
            else
            {
                memcpy(boundsUnion, bounds, 4 * sizeof(float));
                hasBounds = 1;
            }
        }
    }

    if (hasBounds)
        memcpy(result, boundsUnion, 4 * sizeof(float));
}


void glGetPathParameterfvAC(GLuint path, GLenum param, GLfloat *value)
{
    khiter_t iter;
    struct path *p;

    iter = kh_get(path, paths, path);
    if (iter == kh_end(paths))
    {
        /* TODO: check if we should set error here */
        return;
    }

    p = kh_val(paths, iter);

    switch (param)
    {
    case GL_PATH_OBJECT_BOUNDING_BOX_AC:
        break;
    case GL_PATH_FILL_BOUNDING_BOX_AC:
        if (p->is_fill_dirty)
        {
            create_fill_geometry(p);
            p->is_fill_dirty = 0;
        }
        value[0] = p->fill_bounds[0];
        value[1] = p->fill_bounds[1];
        value[2] = p->fill_bounds[2];
        value[3] = p->fill_bounds[3];
        break;
    case GL_PATH_STROKE_BOUNDING_BOX_AC:
        if (p->is_stroke_dirty)
        {
            create_stroke_geometry(p);
            p->is_stroke_dirty = 0;
        }
        value[0] = p->stroke_bounds[0];
        value[1] = p->stroke_bounds[1];
        value[2] = p->stroke_bounds[2];
        value[3] = p->stroke_bounds[3];
        break;
    default:
        /* TODO: check if we should set error here */
        break;
    }

}

void glGetPathParameterivAC(GLuint path, GLenum param, GLint *value)
{


}


void glPathDashArrayAC(GLuint path, GLsizei dashCount, const GLfloat *dashArray)
{
    GLsizei i;

    khiter_t iter;
    struct path *p;

    iter = kh_get(path, paths, path);
    if (iter == kh_end(paths))
    {
        /* TODO: generate error INVALID_OPERATION */
        return;
    }

    p = kh_val(paths, iter);

    if (dashCount < 0)
    {
        /* TODO: generate error INVALID_VALUE */
        return;
    }

    for (i = 0; i < dashCount; ++i)
    {
        if (dashArray[i] < 0)
        {
            /* TODO: generate error INVALID_VALUE */
            return;
        }
    }

    if (dashCount == 0)
    {
        p->num_dashes = 0;
        free(p->dashes);
        p->dashes = NULL;

        p->dash_length = 0;

        p->is_stroke_dirty = 1;
    }
    else
    {
        p->num_dashes = dashCount;
        free(p->dashes);
        p->dashes = malloc(sizeof(float) * dashCount);
        memcpy(p->dashes, dashArray, sizeof(float) * dashCount);

        p->dash_length = 0;
        for (i = 0; i < dashCount; ++i)
            p->dash_length += dashArray[i];

        p->is_stroke_dirty = 1;
    }
}

#ifdef OPENGL_ES
void glLoadPathMatrix(const GLfloat *m)
{
    memcpy(g_mvp, m, 16 * sizeof(GLfloat));
}
void glGetPathMatrix(GLfloat *m)
{
    memcpy(m, g_mvp, 16 * sizeof(GLfloat));
}
#endif

#if 0

static float ccw(const float *p1, const float *p2, const float *p3)
{
    return (p2[0] - p1[0])*(p3[1] - p1[1]) - (p2[1] - p1[1])*(p3[0] - p1[0]);
}

static int compr(const void *ptr1, const void *ptr2)
{
    const float *p1 = (const float*)ptr1;
    const float *p2 = (const float*)ptr2;

    if (p1[0] < p2[0])
        return -1;
    if (p1[0] > p2[0])
        return 1;
    if (p1[1] < p2[1])
        return -1;
    if (p1[1] > p2[1])
        return 1;

    return 0;
}

void evaluate(float Ax, float Ay, float Bx, float By, float Cx, float Cy, float t, float *px, float *py)
{
    *px = Ax * t * t + Bx * t + Cx;
    *py = Ay * t * t + By * t + Cy;
}

void tangent(float Ax, float Ay, float Bx, float By, float Cx, float Cy, float t, float *px, float *py)
{
    float tx = 2 * Ax * t + Bx;
    float ty = 2 * Ay * t + By;

    float l = sqrt(tx * tx + ty * ty);

    tx /= l;
    ty /= l;

    *px = tx;
    *py = ty;
}

void offset(float Ax, float Ay, float Bx, float By, float Cx, float Cy, float t, float d, float *px, float *py)
{
    float tx, ty;
    tangent(Ax, Ay, Bx, By, Cx, Cy, t, &tx, &ty);

    float x, y;
    evaluate(Ax, Ay, Bx, By, Cx, Cy, t, &x, &y);

    *px = x - d * ty;
    *py = y + d * tx;
}

float intersect(float x0, float y0, float tx0, float ty0,
    float x1, float y1, float tx1, float ty1)
{
    /* Solve[x0 + tx0 t0 == x1 + tx1 t1 && y0 + ty0 t0 == y1 + ty1 t1, {t0, t1}] */
    return -((-(ty1*x0) + ty1*x1 + tx1*y0 - tx1*y1) / (tx1*ty0 - tx0*ty1));
}

int convex_hull_quad(float x0, float y0, float x1, float y1, float x2, float y2, float d, float hull[8 * 2 * 2])
{
    int index = 0;
    float vertices[8 * 2];

    float Ax = x0 - 2 * x1 + x2;
    float Ay = y0 - 2 * y1 + y2;
    float Bx = 2 * (x1 - x0);
    float By = 2 * (y1 - y0);
    float Cx = x0;
    float Cy = y0;

    if ((x2 - x0) * (y1 - y0) - (y2 - y0) * (x1 - x0) < 0)
        d = -d;

    /* outer curve */
    float px0, py0, tx0, ty0, ox0, oy0;
    float px1, py1, tx1, ty1, ox1, oy1;

    evaluate(Ax, Ay, Bx, By, Cx, Cy, 0, &px0, &py0);
    tangent(Ax, Ay, Bx, By, Cx, Cy, 0, &tx0, &ty0);
    offset(Ax, Ay, Bx, By, Cx, Cy, 0, d, &ox0, &oy0);

    evaluate(Ax, Ay, Bx, By, Cx, Cy, 1, &px1, &py1);
    tangent(Ax, Ay, Bx, By, Cx, Cy, 1, &tx1, &ty1);
    offset(Ax, Ay, Bx, By, Cx, Cy, 1, d, &ox1, &oy1);

    float m = tan((atan2(ty0, tx0) + atan2(ty1, tx1)) / 2);
    float t = (-By + Bx * m) / (2 * (Ay - Ax * m));                 /* Solve[(2  Ay t + By) / (2 Ax t + Bx) == m, {t}] */

    float pxm, pym, txm, tym, oxm, oym;
    evaluate(Ax, Ay, Bx, By, Cx, Cy, t, &pxm, &pym);
    tangent(Ax, Ay, Bx, By, Cx, Cy, t, &txm, &tym);
    offset(Ax, Ay, Bx, By, Cx, Cy, t, d, &oxm, &oym);

    vertices[index++] = ox0;
    vertices[index++] = oy0;

    vertices[index++] = ox1;
    vertices[index++] = oy1;

    float t0 = intersect(ox0, oy0, tx0, ty0, oxm, oym, txm, tym);
    vertices[index++] = ox0 + tx0 * t0;
    vertices[index++] = oy0 + ty0 * t0;

    float t1 = intersect(ox1, oy1, tx1, ty1, oxm, oym, txm, tym);
    vertices[index++] = ox1 + tx1 * t1;
    vertices[index++] = oy1 + ty1 * t1;

    /* inner curve */
    /*
    px = Ax t^2 + Bx t + Cx
    py = Ay t^2 + By t + Cy
    nx = -D[py, t]
    ny = D[px, t]
    nnx = nx / (nx ^ 2 + ny ^ 2) ^ (1/2)
    nny = ny / (nx ^ 2 + ny ^ 2) ^ (1/2)
    ox = Simplify[px + d * nnx]
    oy = Simplify[py + d * nny]

    Solve[D[ox, t] == 0, {t}]
    */
#define Power pow
#define Sqrt sqrt

    t0 = (4 * Ax*Bx + 4 * Ay*By - Sqrt(Power(-4 * Ax*Bx - 4 * Ay*By, 2) - 4 * (-4 * Power(Ax, 2) - 4 * Power(Ay, 2))*(-Power(Bx, 2) - Power(By, 2) + Power(2, 0.6666666666666666)*Power(Power(Ay*Bx - Ax*By, 2)*Power(d, 2), 0.3333333333333333)))) / (2.*(-4 * Power(Ax, 2) - 4 * Power(Ay, 2)));
    t1 = (4 * Ax*Bx + 4 * Ay*By + Sqrt(Power(-4 * Ax*Bx - 4 * Ay*By, 2) - 4 * (-4 * Power(Ax, 2) - 4 * Power(Ay, 2))*(-Power(Bx, 2) - Power(By, 2) + Power(2, 0.6666666666666666)*Power(Power(Ay*Bx - Ax*By, 2)*Power(d, 2), 0.3333333333333333)))) / (2.*(-4 * Power(Ax, 2) - 4 * Power(Ay, 2)));

    if (0 <= t0 && t0 <= 1)
    {
        float x, y;
        offset(Ax, Ay, Bx, By, Cx, Cy, t0, -d, &x, &y);
        vertices[index++] = x;
        vertices[index++] = y;
    }
    if (0 <= t1 && t1 <= 1)
    {
        float x, y;
        offset(Ax, Ay, Bx, By, Cx, Cy, t1, -d, &x, &y);
        vertices[index++] = x;
        vertices[index++] = y;
    }

#undef Power
#undef Sqrt

    offset(Ax, Ay, Bx, By, Cx, Cy, 0, -d, &ox0, &oy0);
    offset(Ax, Ay, Bx, By, Cx, Cy, 1, -d, &ox1, &oy1);
    vertices[index++] = ox0;
    vertices[index++] = oy0;
    vertices[index++] = ox1;
    vertices[index++] = oy1;

    int npoints = index / 2;

    int num_indices;
    int indices[16];
    {
        qsort(vertices, npoints, sizeof(float)* 2, compr);
        int i, t, k = 0;

        /* lower hull */
        for (i = 0; i < npoints; ++i)
        {
            while (k >= 2 && ccw(&vertices[indices[k - 2] * 2], &vertices[indices[k - 1] * 2], &vertices[i * 2]) <= 0) --k;
            indices[k++] = i;
        }

        /* upper hull */
        for (i = npoints - 2, t = k + 1; i >= 0; --i) {
            while (k >= t && ccw(&vertices[indices[k - 2] * 2], &vertices[indices[k - 1] * 2], &vertices[i * 2]) <= 0) --k;
            indices[k++] = i;
        }

        num_indices = k - 1;
    }

    for (int i = 0; i < num_indices; ++i)
    {
        int index = indices[i];
        hull[i * 2] = vertices[index * 2];
        hull[i * 2 + 1] = vertices[index * 2 + 1];
    }

    return num_indices;
}

/* axis of symmerty */
/*
x = Ax t^2 + Bx t + Cx
y = Ay t^2 + By t + Cy
xp = D[x, t]
xpp = D[xp, t]
yp = D[y, t]
ypp = D[yp, t]
k = ((xp * ypp) - (yp * xpp)) / ((xp^2 + yp^2)^(3/2))
Solve[D[k, t] == 0, {t}]
*/


static const char *fs_code =
"#define M_PI 3.1415926535897932384626433832795                       \n"
"                                                                            \n"
"uniform vec2 A, B, C;                      \n"
"uniform float strokeWidth;                \n"
"uniform float t0, t1;                \n"
"                                                                            \n"
"varying float b, c, d;                    \n"
"varying vec2 pos;          \n"
"                                                                            \n"
"vec2 evaluateQuadratic(float t)                                            \n"
"{                                                                            \n"
"   return (A * t + B) * t + C;                                            \n"
"}                                                                            \n"
"                                                                            \n"
"float f(float t)                                            \n"
"{                                                                            \n"
"   return ((t + b) * t + c) * t + d;                                            \n"
"}                                                                            \n"
"                                                                            \n"
"bool check(float t)                                                        \n"
"{                                                                            \n"
"   if (-1e-3 <= t && t <= 1 + 1e-3)                                                \n"
"   {                                                                        \n"
"      vec2 q = evaluateQuadratic(t) - pos;                                    \n"
"      if (dot(q, q) <= strokeWidth)                                        \n"
"         return false;                                                        \n"
"   }                                                                        \n"
"                                                                            \n"
"   return true;                                                            \n"
"}                                                                            \n"
"                                                                       \n"
"float estimate(vec4 v)                                                       \n"
"{                                                                      \n"
"  float xm = (v.x + v.y) / 2;                                          \n"
"  float ym = f(xm);                                                    \n"
"  float d = (v.y - v.x) / 2;                                           \n"
"  float a = (v.z + v.w - 2 * ym) / (2 * d * d);                        \n"
"  float b = (v.w - v.z) / (2 * d);                                     \n"
"  return xm - (2 * ym) / (b * (1 + sqrt(1 - (4 * a * ym) / (b * b)))); \n"
"}                                                                      \n"
"                                                                       \n"
"vec4 update(vec4 v, float x)                                           \n"
"{                                                                      \n"
"  vec2 m = vec2(x, f(x));                                              \n"
"  vec2 c;                                                              \n"
"  c.x = (sign(v.z * m.y) + 1) / 2;                                     \n"
"  c.y = 1 - c.x;                                                       \n"
"  return c.yxyx * v + c.xyxy * m.xxyy;                                 \n"
"}                                                                      \n"
"                                                                       \n"
"float cbrt(float x)                                                        \n"
"{                                                                            \n"
"   return sign(x) * pow(abs(x), 1.0 / 3.0);                                \n"
"}                                                                            \n"
"// http://stackoverflow.com/questions/11854907/calculate-the-length-of-a-segment-of-a-quadratic-bezier                                                                       \n"
"void dash(float t)                                                                     \n"
"{                                                                       \n"
"                                                                       \n"
"}                                                                       \n"
"                                                                       \n"
"                                                                       \n"
"                                                                       \n"
"                                                                       \n"
"                                                                       \n"
"                                                                       \n"
"void main(void)                                                        \n"
"{                                                                      \n"
#if 1
"  vec4 v = vec4(t0, t1, f(t0), f(t1));                                 \n"
"  v = update(v, estimate(v));                                          \n"
"  v = update(v, estimate(v));                                          \n"
"  v = update(v, estimate(v));                                          \n"
"  float t0 = estimate(v);                                              \n"
"                                                                       \n"
"  float e = (t0 + b) / 2;                                              \n"
"  float f = 2 * e * t0 + c;                                            \n"
"  float disc = e * e - f;                                              \n"
"  if (disc < 0)                                                        \n"
"  {                                                                    \n"
"    if (check(t0)) discard;                                            \n"
"  }                                                                    \n"
"  else                                                                 \n"
"  {                                                                    \n"
"    disc = sqrt(disc);                                                 \n"
"    float t1 = -e - disc;                                              \n"
"    float t2 = -e + disc;                                              \n"
"    if (check(t0) && check(t1) && check(t2)) discard;                  \n"
"  }                                                                    \n"
#else
"  float p = (3 * c - b * b) / 3;                                                  \n"
"  float q = (2 * b * b * b - 9 * b * c + 27 * d) / 27;                            \n"
"  float offset = -b / 3;                                                  \n"
"  float disc = q * q / 4 + p * p * p / 27;                                \n"
"                                                                            \n"
"  if (disc >= 0.0)                                                            \n"
"  {                                                                        \n"
"    float c1 = -q / 2.0;                                                    \n"
"    float c2 = sqrt(disc);                                                    \n"
"    float t0 = cbrt(c1 + c2) + cbrt(c1 - c2) + offset;                                                                      \n"
"    if (check(t0)) discard;                                            \n"
"  }                                                                        \n"
"  else                                                                    \n"
"  {                                                                        \n"
"    float cos_3_theta = 3.0 * q * sqrt(-3.0 / p) / (2.0 * p);            \n"
"    float theta = acos(cos_3_theta) / 3.0;                                \n"
"    float r = 2.0 * sqrt(-p / 3.0);                                        \n"
"    float t0 = r * cos(theta) + offset;                                                                        \n"
"    float t1 = r * cos(theta + 2.0 * M_PI / 3.0) + offset;                                                                        \n"
"    float t2 = r * cos(theta + 4.0 * M_PI / 3.0) + offset;                                                                        \n"
"    if (check(t0) && check(t1) && check(t2)) discard;                  \n"
"   }                                                                        \n"
#endif
"  gl_FragColor = vec4(1, 1, 1, 1);                                         \n"
"}                                                                      \n";


static void clip_poly(const float *poly, int npoly,
    float px, float py, float nx, float ny,
    float *poly0, int *npoly0,
    float *poly1, int *npoly1)
{
    int index0 = 0;
    int index1 = 0;

    for (int i = 0; i < npoly; ++i)
    {
        int i1 = (i + 1) % npoly;

        double x0 = poly[i * 2];
        double y0 = poly[i * 2 + 1];
        double x1 = poly[i1 * 2];
        double y1 = poly[i1 * 2 + 1];

        bool b0 = ((x0 - px) * ny - (y0 - py) * nx) >= 0;
        bool b1 = ((x1 - px) * ny - (y1 - py) * nx) >= 0;

        float xp, yp;
        if (b0 != b1)
        {
            float t = intersect(x0, y0, x1 - x0, y1 - y0, px, py, nx, ny);
            xp = (1 - t) * x0 + t * x1;
            yp = (1 - t) * y0 + t * y1;
        }

        if (!b0 && b1)
        {
            // out (xp, yp, x1, y1)
            // out (xp, yp)
            poly0[index0++] = xp;
            poly0[index0++] = yp;
            poly0[index0++] = x1;
            poly0[index0++] = y1;
            poly1[index1++] = xp;
            poly1[index1++] = yp;
        }
        else if (b0 && b1)
        {
            // out (x1, y1)
            // out none
            poly0[index0++] = x1;
            poly0[index0++] = y1;
        }
        else if (b0 && !b1)
        {
            // out (xp, yp)
            // out (xp, yp, x1, y1)
            poly0[index0++] = xp;
            poly0[index0++] = yp;
            poly1[index1++] = xp;
            poly1[index1++] = yp;
            poly1[index1++] = x1;
            poly1[index1++] = y1;
        }
        else if (!b0 && !b1)
        {
            // out none
            // out (x1, y1) 
            poly1[index1++] = x1;
            poly1[index1++] = y1;
        }
    }

    *npoly0 = index0 / 2;
    *npoly1 = index1 / 2;

}


#endif
