package com.example.iot_control

import android.Manifest
import android.app.NotificationChannel
import android.app.NotificationManager
import android.content.Context
import android.content.pm.PackageManager
import android.os.Build
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.activity.result.contract.ActivityResultContracts
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import com.example.iot_control.presentation.DashboardScreen

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        setContent {
            DashboardScreen()
        }
    }
//    fun requestNotificationPermission() {
//            when {
//                ContextCompat.checkSelfPermission(
//                    this,
//                    Manifest.permission.POST_NOTIFICATIONS
//                ) == PackageManager.PERMISSION_GRANTED -> {
//                    // Permission already granted
//                }
//                ActivityCompat.shouldShowRequestPermissionRationale(
//                    this,
//                    Manifest.permission.POST_NOTIFICATIONS
//                ) -> {
//                    // Explain why you need the permission, then launch request
//                    showRationaleUI()
//                }
//                else -> {
//                    // Directly launch the permission request
//                    requestPermissionLauncher.launch(Manifest.permission.POST_NOTIFICATIONS)
//                }
//            }
//
//    }
//    private val requestPermissionLauncher = registerForActivityResult(
//        ActivityResultContracts.RequestPermission()
//    ) { isGranted: Boolean ->
//        if (isGranted) {
//            // Permission granted, you can post notifications.
//        } else {
//            // Permission denied. Handle gracefully.
//        }
//    }

    private fun createNotificationChannel() {
        val channel = NotificationChannel(
            "notification_channel_id", // Same ID used in NotificationHandler
            "WebSocket Messages",     // User-visible name
            NotificationManager.IMPORTANCE_HIGH // Importance level
        ).apply {
            description = "Notifications for incoming WebSocket messages"
        }
        // Register the channel with the system
        val notificationManager =
            getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager
        notificationManager.createNotificationChannel(channel)

//        val channelFg = NotificationChannel(
//            "iot_websocket_service", // Same ID used in NotificationHandler
//            "WebSocket Messages",     // User-visible name
//            NotificationManager.IMPORTANCE_HIGH // Importance level
//        ).apply {
//            description = "Notifications for incoming WebSocket messages"
//        }
//        // Register the channel with the system
//        val notificationManagerFg =
//            getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager
//        notificationManagerFg.createNotificationChannel(channelFg)
    }

}