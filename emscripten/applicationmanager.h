#include <vector>
#include <luaapplication.h>

struct ProjectProperties
{
    ProjectProperties()
        {
                scaleMode = 0;
                        logicalWidth = 320;
                                logicalHeight = 480;
                                        orientation = 0;
                                                fps = 60;
                                                        retinaDisplay = 0;
                                                                autorotation = 0;
                                                                        mouseToTouch = 1;
                                                                                touchToMouse = 1;
                                                                                        mouseTouchOrder = 0;
                                                                                            }
                                                                                            
                                                                                                int scaleMode;
                                                                                                    int logicalWidth;
                                                                                                        int logicalHeight;
                                                                                                            std::vector<std::pair<std::string, float> > imageScales;
                                                                                                                int orientation;
                                                                                                                    int fps;
                                                                                                                        int retinaDisplay;
                                                                                                                            int autorotation;
                                                                                                                                int mouseToTouch;
                                                                                                                                    int touchToMouse;
                                                                                                                                        int mouseTouchOrder;
                                                                                                                                        };
class ApplicationManager
{
public:
  ApplicationManager(bool player);
    ~ApplicationManager();
    
      void luaError(const char *msg);
        
          void surfaceCreated();
            void surfaceChanged(int width, int height, int rotation);
              void updateHardwareOrientation();
                void drawFrame();
                
                  void setDirectories(const char *externalDir, const char *internalDir, const char *cacheDir);
                    void setFileSystem(const char *files);
                      
                        void openProject(const char* project);
                          void setOpenProject(const char* project);
                            void play(const std::vector<std::string>& luafiles);
                              void stop();
                                void setProjectName(const char *projectName);
                                  void setProjectProperties(const ProjectProperties &properties);
                                    bool isRunning();
                                      
                                        void touchesBegin(int size, int *id, int *x, int *y, int actionIndex);
                                          void touchesMove(int size, int *id, int *x, int *y);
                                            void touchesEnd(int size, int *id, int *x, int *y, int actionIndex);
                                              void touchesCancel(int size, int *id, int *x, int *y);
                                                
                                                  bool keyDown(int keyCode, int repeatCount);
                                                    bool keyUp(int keyCode, int repeatCount);
                                                      
                                                        void pause();
                                                          void resume();
                                                          
                                                            void lowMemory();
                                                            
                                                              void background();
                                                                void foreground();	
                                                                  
                                                                  private:
                                                                    void loadProperties();
                                                                      void loadLuaFiles();
                                                                        void drawIPs();
                                                                          int convertKeyCode(int keyCode);
                                                                            
                                                                            private:
                                                                              bool player_;
                                                                                LuaApplication *application_;
                                                                                    
                                                                                      bool init_;
                                                                                        
                                                                                          bool running_;
                                                                                            
                                                                                              int width_, height_;
                                                                                              
//                                                                                                SplashScreen *splashScreen_;
                                                                                                  
                                                                                                    std::string externalDir_, internalDir_, cacheDir_;
                                                                                                      
                                                                                                        ProjectProperties properties_;
                                                                                                          
                                                                                                            Orientation hardwareOrientation_;
                                                                                                              
                                                                                                                Orientation deviceOrientation_;
                                                                                                                  
                                                                                                                    int nframe_;
                                                                                                                      
                                                                                                                        bool applicationStarted_;
                                                                                                                          
                                                                                                                            bool skipFirstEnterFrame_;
                                                                                                                            };
                                                                                                                            