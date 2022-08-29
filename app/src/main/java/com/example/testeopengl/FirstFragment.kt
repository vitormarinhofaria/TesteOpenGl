package com.example.testeopengl

import android.content.ContentResolver
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.net.Uri
import android.os.Build
import android.os.Bundle
import android.os.Environment
import android.os.ParcelFileDescriptor
import android.util.Log
import androidx.fragment.app.Fragment
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.activity.result.contract.ActivityResultContracts
import androidx.annotation.RequiresApi
import com.example.testeopengl.databinding.FragmentFirstBinding
import java.io.FileDescriptor
import java.lang.annotation.Native

/**
 * A simple [Fragment] subclass as the default destination in the navigation.
 */
private const val OPEN_DOCUMENT_REQUEST_CODE = 0x33
class FirstFragment : Fragment() {

    private var _binding: FragmentFirstBinding? = null

    // This property is only valid between onCreateView and
    // onDestroyView.
    private val binding get() = _binding!!
    private var renderer: NativeRenderer? = null
    override fun onCreateView(
        inflater: LayoutInflater, container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {

        _binding = FragmentFirstBinding.inflate(inflater, container, false)
        return binding.root

    }
    private val getPreviewImage = registerForActivityResult(ActivityResultContracts.OpenDocument()) {
        Log.d("FILE_PICK", it.toString())
        if(it != null){
            val bitmap = getBitmapFromUri(it)
            Log.d("FILE_PICK","Height: ${bitmap.height} Widht: ${bitmap.width}")
            binding.glPlace.queueEvent{
                NativeRenderer.setTexture(bitmap);
            }

        }
    }

    @RequiresApi(Build.VERSION_CODES.R)
    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)

        binding.buttonFirst.setOnClickListener {
            getPreviewImage.launch(arrayOf("image/*"))
            //findNavController().navigate(R.id.action_FirstFragment_to_SecondFragment)
        }

        binding.btnSave.setOnClickListener { NativeRenderer.takePrint() }

        val internalStorage = Environment.getStorageDirectory().absolutePath;
        val externalStorage = Environment.getExternalStorageDirectory().absolutePath;
        val dataStorage = Environment.getDataDirectory().absolutePath;
        val rootStorage = Environment.getRootDirectory().absolutePath;
        NativeRenderer.setBasePath(externalStorage);
        val width = binding.glPlace.width
        val height = binding.glPlace.height
        NativeRenderer.SetScreenResolution(width, height)
        //val res = resources.getIdentifier("epic_orchestra_loop", "raw", context?.packageName)
        val res = resources.getIdentifier("epic", "raw", context?.packageName)
        val music = resources.openRawResource(res)
        //val music = resources.assets.open("res:raw/epic_orchestra_loop.wav")
        val bytes = music.readBytes();
        NativeRenderer.saveMusic(bytes);

        renderer = NativeRenderer()
        binding.glPlace.setRenderer(renderer)
        contentResolver = context?.contentResolver
    }

    private var contentResolver: ContentResolver? = null

    private fun getBytesFromUri(uri: Uri): List<Byte> {
        val parcelFileDescriptor: ParcelFileDescriptor? =
            contentResolver?.openFileDescriptor(uri, "r")
        val f = contentResolver?.openFile(uri, "r", null)

        val inputStream = ParcelFileDescriptor.AutoCloseInputStream(parcelFileDescriptor)
        return inputStream.readBytes().toList()
    }
    private fun getBitmapFromUri(uri: Uri): Bitmap {
        val parcelFileDescriptor: ParcelFileDescriptor? =
            contentResolver?.openFileDescriptor(uri, "r")
        val fileDescriptor: FileDescriptor = parcelFileDescriptor!!.fileDescriptor

        val image: Bitmap = BitmapFactory.decodeFileDescriptor(fileDescriptor)
        parcelFileDescriptor.close()
        return image
    }

    override fun onDestroyView() {
        super.onDestroyView()
        _binding = null
    }
}