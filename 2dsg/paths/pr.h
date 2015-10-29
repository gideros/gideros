#ifndef PR_H_INCLUDED
#define PR_H_INCLUDED

#include <gl/glew.h>

#define OPENGL_ES

#define GL_CLOSE_PATH_AC                                   0x00
#define GL_MOVE_TO_AC                                      0x02
#define GL_RELATIVE_MOVE_TO_AC                             0x03
#define GL_LINE_TO_AC                                      0x04
#define GL_RELATIVE_LINE_TO_AC                             0x05
#define GL_HORIZONTAL_LINE_TO_AC                           0x06
#define GL_RELATIVE_HORIZONTAL_LINE_TO_AC                  0x07
#define GL_VERTICAL_LINE_TO_AC                             0x08
#define GL_RELATIVE_VERTICAL_LINE_TO_AC                    0x09
#define GL_QUADRATIC_CURVE_TO_AC                           0x0A
#define GL_RELATIVE_QUADRATIC_CURVE_TO_AC                  0x0B
#define GL_CUBIC_CURVE_TO_AC                               0x0C
#define GL_RELATIVE_CUBIC_CURVE_TO_AC                      0x0D
#define GL_SMOOTH_QUADRATIC_CURVE_TO_AC                    0x0E
#define GL_RELATIVE_SMOOTH_QUADRATIC_CURVE_TO_AC           0x0F
#define GL_SMOOTH_CUBIC_CURVE_TO_AC                        0x10
#define GL_RELATIVE_SMOOTH_CUBIC_CURVE_TO_AC               0x11
#define GL_SMALL_CCW_ARC_TO_AC                             0x12
#define GL_RELATIVE_SMALL_CCW_ARC_TO_AC                    0x13
#define GL_SMALL_CW_ARC_TO_AC                              0x14
#define GL_RELATIVE_SMALL_CW_ARC_TO_AC                     0x15
#define GL_LARGE_CCW_ARC_TO_AC                             0x16
#define GL_RELATIVE_LARGE_CCW_ARC_TO_AC                    0x17
#define GL_LARGE_CW_ARC_TO_AC                              0x18
#define GL_RELATIVE_LARGE_CW_ARC_TO_AC                     0x19
#define GL_RESTART_PATH_AC                                 0xF0
#define GL_DUP_FIRST_CUBIC_CURVE_TO_AC                     0xF2
#define GL_DUP_LAST_CUBIC_CURVE_TO_AC                      0xF4
#define GL_RECT_AC                                         0xF6
#define GL_CIRCULAR_CCW_ARC_TO_AC                          0xF8
#define GL_CIRCULAR_CW_ARC_TO_AC                           0xFA
#define GL_CIRCULAR_TANGENT_ARC_TO_AC                      0xFC
#define GL_ARC_TO_AC                                       0xFE
#define GL_RELATIVE_ARC_TO_AC                              0xFF
#define GL_PATH_STROKE_WIDTH_AC                            0x9075
#define GL_PATH_INITIAL_END_CAP_AC                         0x9077
#define GL_PATH_TERMINAL_END_CAP_AC                        0x9078
#define GL_PATH_JOIN_STYLE_AC                              0x9079
#define GL_PATH_MITER_LIMIT_AC                             0x907A
#define GL_COUNT_UP_AC                                     0x9088
#define GL_COUNT_DOWN_AC                                   0x9089
#define GL_CONVEX_HULL_AC                                  0x908B
#define GL_BOUNDING_BOX_AC                                 0x908D
#define GL_SQUARE_AC                                       0x90A3
#define GL_ROUND_AC                                        0x90A4
#define GL_TRIANGULAR_AC                                   0x90A5
#define GL_BEVEL_AC                                        0x90A6
#define GL_MITER_REVERT_AC                                 0x90A7
#define GL_MITER_TRUNCATE_AC                               0x90A8
#define GL_UTF8_AC                                         0x909A
#define GL_UTF16_AC                                        0x909B
#define GL_TRANSLATE_X_AC                                  0x908E
#define GL_TRANSLATE_Y_AC                                  0x908F
#define GL_TRANSLATE_2D_AC                                 0x9090
#define GL_TRANSLATE_3D_AC                                 0x9091
#define GL_AFFINE_2D_AC                                    0x9092
#define GL_AFFINE_3D_AC                                    0x9094
#define GL_TRANSPOSE_AFFINE_2D_AC                          0x9096
#define GL_TRANSPOSE_AFFINE_3D_AC                          0x9098
#define GL_PATH_OBJECT_BOUNDING_BOX_AC                     0x908A
#define GL_PATH_FILL_BOUNDING_BOX_AC                       0x90A1
#define GL_PATH_STROKE_BOUNDING_BOX_AC                     0x90A2

#ifndef GL_2_BYTES
#define GL_2_BYTES                                         0x1407
#endif

#ifndef GL_3_BYTES
#define GL_3_BYTES                                         0x1408
#endif

#ifndef GL_4_BYTES
#define GL_4_BYTES                                         0x1409
#endif

#ifdef __cplusplus
extern "C" {
#endif
    
void init_path_rendering();
void cleanup_path_rendering();

GLuint glGenPathsAC(GLsizei range);
void glDeletePathsAC(GLuint path, GLsizei range);
GLboolean glIsPathAC(GLuint path);
void glPathCommandsAC(GLuint path, GLsizei numCommands, const GLubyte *commands, GLsizei numCoords, GLenum coordType, const GLvoid *coords);
void glStencilStrokePathAC(GLuint path, GLint reference, GLuint mask);
void glStencilFillPathAC(GLuint path, GLenum fillMode, GLuint mask);
void glPathParameterfAC(GLuint path, GLenum pname, GLfloat value);
void glPathParameteriAC(GLuint path, GLenum pname, GLint value);

void glStencilFillPathInstancedAC(GLsizei numPaths, GLenum pathNameType, const void* paths, GLuint pathBase, GLenum fillMode, GLuint mask, GLenum transformType, const GLfloat *transformValues);
void glStencilStrokePathInstancedAC(GLsizei numPaths, GLenum pathNameType, const void* paths, GLuint pathBase, GLint reference, GLuint mask, GLenum transformType, const GLfloat *transformValues);

void glGetPathBoundingBoxInstancedAC(GLenum param, GLsizei numPaths, GLenum pathNameType, const void* paths, GLuint pathBase, GLenum transformType, const GLfloat *transformValues, GLfloat *bounds);

void glGetPathParameterfvAC(GLuint path, GLenum param, GLfloat *value);
void glGetPathParameterivAC(GLuint path, GLenum param, GLint *value);

void glPathDashArrayAC(GLuint path, GLsizei dashCount, const GLfloat *dashArray);

#ifdef OPENGL_ES
void glLoadPathMatrix(const GLfloat *m);
void glGetPathMatrix(GLfloat *m);
#endif

#ifdef __cplusplus
}
#endif

#endif
