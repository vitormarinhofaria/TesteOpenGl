package com.example.testeopengl

import android.Manifest.permission.MANAGE_EXTERNAL_STORAGE
import android.content.Context
import android.content.Intent
import android.content.pm.PackageManager
import android.media.AudioManager
import android.media.AudioManager.GET_DEVICES_OUTPUTS
import android.media.MediaCodec
import android.media.MediaCodecInfo
import android.net.Uri
import android.os.Build
import android.os.Bundle
import android.os.Environment
import android.provider.Settings
import android.util.Log
import android.view.Menu
import android.view.MenuItem
import android.widget.Toast
import androidx.annotation.RequiresApi
import androidx.appcompat.app.AppCompatActivity
import androidx.navigation.findNavController
import androidx.navigation.ui.AppBarConfiguration
import androidx.navigation.ui.navigateUp
import androidx.navigation.ui.setupActionBarWithNavController
import com.arthenica.ffmpegkit.FFmpegKit
import com.example.testeopengl.databinding.ActivityMainBinding
import com.google.android.material.snackbar.Snackbar


class MainActivity : AppCompatActivity() {
    companion object{
        init {
            System.loadLibrary("testeopengl")
            System.loadLibrary("andr_lib")
        }
    }

    private lateinit var appBarConfiguration: AppBarConfiguration
    private lateinit var binding: ActivityMainBinding

    override fun onRequestPermissionsResult(
        requestCode: Int,
        permissions: Array<out String>,
        grantResults: IntArray
    ) {
        if(requestCode == 0){
            if(grantResults.size == 1 && grantResults[0] == PackageManager.PERMISSION_GRANTED){
                Toast.makeText(applicationContext, "Permission Granted", Toast.LENGTH_LONG).show()
            }else{
                Toast.makeText(applicationContext, "Permission Denied", Toast.LENGTH_LONG).show()
            }
        }
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
    }


    @RequiresApi(Build.VERSION_CODES.R)
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
//        if (!Environment.isExternalStorageManager()) {
//            val intent = Intent()
//            intent.action = Settings.ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION
//            val uri: Uri = Uri.fromParts("package", this.packageName, null)
//            intent.data = uri
//            startActivity(intent)
//        }

        if (checkSelfPermission(MANAGE_EXTERNAL_STORAGE) == PackageManager.PERMISSION_DENIED
        ) {
            requestPermissions(
                arrayOf(
                    MANAGE_EXTERNAL_STORAGE
                ), 0
            )
        } else {
            Log.d("PERMISSION", "Manage Permission Granted")
        }

        val audioManager = getSystemService(Context.AUDIO_SERVICE) as AudioManager
        val audioDevices = audioManager.getDevices(GET_DEVICES_OUTPUTS)

        //NativeRenderer.CreateAudioWithDeviceId(audioDevices[2].id)
        NativeRenderer.CreateAudioWithDeviceId(0)
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        setSupportActionBar(binding.toolbar)

        val navController = findNavController(R.id.nav_host_fragment_content_main)
        appBarConfiguration = AppBarConfiguration(navController.graph)
        setupActionBarWithNavController(navController, appBarConfiguration)

        binding.fab.setOnClickListener { view ->
            Snackbar.make(view, "Replace with your own action", Snackbar.LENGTH_LONG)
                .setAction("Action", null).show()
        }

        val codec = MediaCodec.createEncoderByType("video/avc")
        val types = codec.codecInfo.supportedTypes
        types.forEach {
            val codecName = it
            val cap = codec.codecInfo.getCapabilitiesForType(it)
            cap.colorFormats.forEach {
                Log.i("CODEC_INFO", "Codec $codecName -> $it")
            }
        }
        Log.i("CODEC_INFO", "")
    }

    override fun onCreateOptionsMenu(menu: Menu): Boolean {
        // Inflate the menu; this adds items to the action bar if it is present.
        menuInflater.inflate(R.menu.menu_main, menu)
        return true
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        return when (item.itemId) {
            R.id.action_settings -> true
            else -> super.onOptionsItemSelected(item)
        }
    }

    override fun onSupportNavigateUp(): Boolean {
        val navController = findNavController(R.id.nav_host_fragment_content_main)
        return navController.navigateUp(appBarConfiguration)
                || super.onSupportNavigateUp()
    }
}