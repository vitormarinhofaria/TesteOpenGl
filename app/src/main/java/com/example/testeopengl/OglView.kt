package com.example.testeopengl

import android.content.Context
import android.graphics.Canvas
import android.graphics.Color
import android.graphics.Paint
import android.graphics.drawable.Drawable
import android.opengl.GLSurfaceView
import android.text.TextPaint
import android.util.AttributeSet
import android.view.View
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

/**
 * TODO: document your custom view class.
 */
class OglView : GLSurfaceView {
    constructor(context: Context) : super(context){

    }
    constructor(context: Context, attributeSet: AttributeSet) : super(context, attributeSet){

    }
    init {
        setEGLContextClientVersion(3)
        preserveEGLContextOnPause = true
    }
}

class Vec3(var r: Float = 0.0F, var g: Float = 0.0F, var b: Float = 0.0F) {

}

class MyRenderer : GLSurfaceView.Renderer{
    var color: Vec3 = Vec3()

    override fun onSurfaceCreated(gl: GL10, config: EGLConfig) {
    }

    override fun onSurfaceChanged(gl: GL10, width: Int, height: Int) {

    }

    override fun onDrawFrame(gl: GL10) {
        if (color.r >= 1.0F){
            color.r = 0.0F
        }else{
            color.r += 0.01F
        }
        gl.glClearColor(color.r, color.g, color.b, 1.0F)
        gl.glClear(GL10.GL_COLOR_BUFFER_BIT)
    }

}