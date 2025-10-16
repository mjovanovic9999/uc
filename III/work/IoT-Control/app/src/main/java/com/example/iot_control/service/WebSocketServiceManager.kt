package com.example.iot_control.service


import android.app.NotificationManager
import android.content.Context
import android.content.Intent
import com.example.iot_control.core.Constants.EXTRA_WS_SERVER_URL

class WebSocketServiceManager(private val context: Context) {
    private var isRunning = false
    fun startService(serverUrl: String) {
        isRunning = true
        val intent = Intent(context, WebSocketForegroundService::class.java).apply {
            putExtra(EXTRA_WS_SERVER_URL, serverUrl)
        }
        context.startForegroundService(intent)
    }

    fun stopService() {
        isRunning = false
        val intent = Intent(context, WebSocketForegroundService::class.java)
        context.stopService(intent)
    }

//    fun isServiceRunning(): Boolean = isRunning
fun isServiceRunning(context: Context): Boolean {
    val notificationManager =
        context.getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager
    val activeNotifications = notificationManager.activeNotifications
    return activeNotifications.any { it.id == 1 }
}
}
