#ifndef NGLSCENE_H_
#define NGLSCENE_H_

#include "WindowParams.h"
#include <ngl/Camera.h>
#include <ngl/Colour.h>
#include <ngl/Transformation.h>
#include <ngl/Light.h>
#include <ngl/Material.h>
#include <Axis.h>
#include <QOpenGLWidget>
#include <memory>
//----------------------------------------------------------------------------------------------------------------------
/// @file NGLScene.h
/// @brief a basic Qt GL window class for ngl demos
/// @author Jonathan Macey
/// @version 1.0
/// @date 10/10/10
/// Revision History :
/// Initial Version 10/10/10 (Binary day ;-0 )
/// @class GLWindow
/// @brief our main glwindow widget for NGL applications all drawing elements are
/// put in this file
//----------------------------------------------------------------------------------------------------------------------
class NGLScene : public QOpenGLWidget
{
Q_OBJECT        // must include this if you use Qt signals/slots
public :
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief Constructor for GLWindow
  /// @param [in] _parent the parent window to create the GL context in
  //----------------------------------------------------------------------------------------------------------------------
  NGLScene(QWidget *_parent );
  //----------------------------------------------------------------------------------------------------------------------
  //----------------------------------------------------------------------------------------------------------------------
  void resetMouse();
private :

  //----------------------------------------------------------------------------------------------------------------------
  /// @brief used to store the x rotation mouse value
  //----------------------------------------------------------------------------------------------------------------------
  WinParams m_win;
  //----------------------------------------------------------------------------------------------------------------------
  ngl::Vec3 m_modelPos;
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief used to store the global mouse transforms
  //----------------------------------------------------------------------------------------------------------------------
  ngl::Mat4 m_mouseGlobalTX;
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief Our Camera
  //----------------------------------------------------------------------------------------------------------------------
  ngl::Camera m_cam;
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief the index into the VBO name array for the drawing
  //----------------------------------------------------------------------------------------------------------------------
  int m_drawIndex;
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief flag to indicate if we draw the normals
  //----------------------------------------------------------------------------------------------------------------------
  bool m_drawNormals;
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief the final transform matrix
  //----------------------------------------------------------------------------------------------------------------------
  ngl::Mat4 m_transform;
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief a matrix to hold the rotation
  //----------------------------------------------------------------------------------------------------------------------
  ngl::Mat4 m_rotate;

  //----------------------------------------------------------------------------------------------------------------------
  /// @brief a matrix to demonstrate gimal locs
  //----------------------------------------------------------------------------------------------------------------------
  ngl::Mat4 m_gimbal;
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief the translation matrix
  //----------------------------------------------------------------------------------------------------------------------
  ngl::Mat4 m_translate;
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief the scale matrix
  //----------------------------------------------------------------------------------------------------------------------
  ngl::Mat4 m_scale;
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief the Euler rotation matrix
  //----------------------------------------------------------------------------------------------------------------------
  ngl::Mat4 m_euler;
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief the colour for the object material
  //----------------------------------------------------------------------------------------------------------------------
  ngl::Colour m_colour;
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief the size to draw the normals
  //----------------------------------------------------------------------------------------------------------------------
  int m_normalSize;
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief the axis
  //----------------------------------------------------------------------------------------------------------------------
  std::unique_ptr<Axis> m_axis;
  //----------------------------------------------------------------------------------------------------------------------
  //----------------------------------------------------------------------------------------------------------------------
  ngl::Material m_material;
  bool m_wireframe;
  //----------------------------------------------------------------------------------------------------------------------
  /// @enum for the Matrix order Rotate Trans Scale, Trans Rotate Scale or Euler
  //----------------------------------------------------------------------------------------------------------------------
  enum class MatrixOrder{
                    RTS, ///<Rotate Translate Scale
                    TRS, ///<Translate Rotate Scale
                    GIMBALLOCK,
                    EULERTS, //< Use Axis Angle Euler Trans Scale
                    TEULERS //<  Use Translate Euler Scale

                  };
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief the order of multiplication for the transform matrix
  //----------------------------------------------------------------------------------------------------------------------
  MatrixOrder m_matrixOrder;

public slots :
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief slot to indicate the vbo selection combo box has changed
  /// called from MainWindow
  /// @param[in] _index the new value of the combo box index
  //----------------------------------------------------------------------------------------------------------------------
  void vboChanged(int _index);
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief slot to indicate the normal tick box has been activated
  /// called from MainWindow
  /// @param[in] _value the new value of the tick box
  //----------------------------------------------------------------------------------------------------------------------
  void toggleNormals(bool _value );
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief slot to indicate the wireframe tick box has been activated
  /// called from MainWindow
  /// @param[in] _value the new value of the tick box
  //----------------------------------------------------------------------------------------------------------------------
  void toggleWireframe(bool _value ){m_wireframe=_value; update();}
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief slot to indicate the normal length slider had changed
  /// called from MainWindow
  /// @param[in] _value the new value of the tick box
  //----------------------------------------------------------------------------------------------------------------------
  void setNormalSize(int _value );
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief called when any of the scale elements are modified sets the
  /// new m_scale matrix value and forces a re-calcuation and re-draw
  /// called from MainWindow
  /// @param[in] _x the value of scale in the x
  /// @param[in] _y the value of scale in the y
  /// @param[in] _z the value of scale in the z
  //----------------------------------------------------------------------------------------------------------------------
  void setScale(float _x,float _y, float _z );
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief called when any of the translate elements are modified sets the
  /// new m_translate matrix value and forces a re-calcuation and re-draw
  /// called from MainWindow
  /// @param[in] _x the value of translate in the x
  /// @param[in] _y the value of translate in the y
  /// @param[in] _z the value of translate in the z
  //----------------------------------------------------------------------------------------------------------------------

  void setTranslate(float _x,float _y,float _z);
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief called when any of the rotate elements are modified sets the
  /// new m_rotate matrix value and forces a re-calcuation and re-draw
  /// called from MainWindow
  /// @param[in] _x the value of rotation in the x
  /// @param[in] _y the value of rotation in the y
  /// @param[in] _z the value of rotation in the z
  //----------------------------------------------------------------------------------------------------------------------
  void setRotate(float _x,float _y, float _z );
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief called when the colour button is pressed and a new colour selected
  /// sets a new current material and forces update
  /// called from MainWindow
  /// @param[in] _r the value of red
  /// @param[in] _g the value of green
  /// @param[in] _b the value of blue
  //----------------------------------------------------------------------------------------------------------------------
  void setColour(float _r,float _g,float _b );
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief called when the matrix order combobox is changed sets the new
  /// matrix order for m_matrixOrder and forces re-draw
  /// called from MainWindow
  /// @param[in] _index the index of the combobox
  //---------------------------------------------------------------------------------------------------------------------
  void setMatrixOrder( int _index);
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief called when any of the euler rotation elements are modified sets the
  /// new m_euler matrix value and forces a re-calcuation and re-draw
  /// called from MainWindow
  /// @param[in] _angle the angle in degrees
  /// @param[in] _x the value of rotation axis [-1 , 1]
  /// @param[in] _y the value of rotation axis [-1 , 1]
  /// @param[in] _z the value of rotation axis [-1 , 1]
  //----------------------------------------------------------------------------------------------------------------------
  void setEuler(float _angle,float _x,float _y,float _z );

 signals :
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief signal emitted when the transform matrix is modified
  /// it is recived by the main window which will set the main matrix values
  /// @param _m the new transformation values used in the display
  //----------------------------------------------------------------------------------------------------------------------
  void matrixDirty(ngl::Mat4 _m);
protected:

  //----------------------------------------------------------------------------------------------------------------------
  /// @brief  The following methods must be implimented in the sub class
  /// this is called when the window is created
  //----------------------------------------------------------------------------------------------------------------------
  void initializeGL();

  //----------------------------------------------------------------------------------------------------------------------
  /// @brief this is called whenever the window is re-sized
  /// @param[in] _w the width of the resized window
  /// @param[in] _h the height of the resized window
  //----------------------------------------------------------------------------------------------------------------------
  void resizeGL(const int _w, const int _h  );
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief this is the main gl drawing routine which is called whenever the window needs to
  /// be re-drawn
  //----------------------------------------------------------------------------------------------------------------------
  void paintGL();

private :
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief this method is called every time a mouse is moved
  /// @param _event the Qt Event structure
  //----------------------------------------------------------------------------------------------------------------------
  void mouseMoveEvent ( QMouseEvent * _event );
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief this method is called everytime the mouse button is pressed
  /// inherited from QObject and overridden here.
  /// @param _event the Qt Event structure
  //----------------------------------------------------------------------------------------------------------------------

  void mousePressEvent (QMouseEvent *_event );

  //----------------------------------------------------------------------------------------------------------------------
  /// @brief this method is called everytime the mouse button is released
  /// inherited from QObject and overridden here.
  /// @param _event the Qt Event structure
  //----------------------------------------------------------------------------------------------------------------------
  void mouseReleaseEvent (QMouseEvent *_event);
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief this method is called everytime the mouse wheel is moved
  /// inherited from QObject and overridden here.
  /// @param _event the Qt Event structure
  //----------------------------------------------------------------------------------------------------------------------
  void wheelEvent(QWheelEvent *_event );

  void loadMatricesToShader();



};

#endif
