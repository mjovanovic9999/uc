package com.example.iot_control.service


import android.content.Context
import android.content.Intent
import android.util.Log

class WebSocketServiceManager(private val context: Context) {
    fun startService(serverUrl: String) {
        Log.d("wsmngr","started")
        val intent = Intent(context, WebSocketForegroundService::class.java).apply {
            putExtra("server_url", serverUrl)
        }
        context.startForegroundService(intent)
    }

    fun stopService() {
        val intent = Intent(context, WebSocketForegroundService::class.java)
        context.stopService(intent)
    }

    fun isServiceRunning(): Boolean {
        // You can implement this by tracking service state
        return true // Simplified - implement proper check
    }
}