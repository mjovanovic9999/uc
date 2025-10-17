package com.example.iot_control.service

import android.annotation.SuppressLint
import android.app.AlarmManager
import android.app.Notification
import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.PendingIntent
import android.app.Service
import android.content.Context
import android.content.Intent
import android.content.pm.ServiceInfo
import android.os.Binder
import android.os.IBinder
import android.os.SystemClock
import android.util.Log
import androidx.core.app.NotificationCompat
import com.example.iot_control.MainActivity
import com.example.iot_control.R
import com.example.iot_control.core.Constants.EXTRA_WS_SERVER_URL
import com.example.iot_control.core.Constants.WS_URL
import com.example.iot_control.core.WebSocketManager
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlin.random.Random

class WebSocketForegroundService : Service() {
    private val binder = LocalBinder()
    private var webSocketManager: WebSocketManager? = null
    private val _isConnected = MutableStateFlow(false)
    val isConnected: StateFlow<Boolean> = _isConnected.asStateFlow()
    private val _lastMessage = MutableStateFlow<String?>(null)
    val lastMessage: StateFlow<String?> = _lastMessage.asStateFlow()

    private lateinit var notificationManager: NotificationManager

    inner class LocalBinder : Binder() {
        fun getService(): WebSocketForegroundService = this@WebSocketForegroundService
    }

    override fun onBind(intent: Intent?): IBinder = binder

    override fun onCreate() {
        super.onCreate()
        notificationManager = getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager
        createServiceNotificationChannel()
        createMessageNotificationChannel()

        Log.d("WSService", "Service created")
    }

    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        Log.d("WSService", "onStartCommand called")

        val serverUrl = intent?.getStringExtra(EXTRA_WS_SERVER_URL) ?: WS_URL

        try {
            val notification = createNotification("Starting IoT service...")
            startForeground(
                NOTIFICATION_ID,
                notification,
                ServiceInfo.FOREGROUND_SERVICE_TYPE_DATA_SYNC
            )

            Log.d("WSService", "Started in foreground")
        } catch (e: Exception) {
            Log.e("WSService", "Failed to start foreground", e)
            stopSelf()
            return START_NOT_STICKY
        }

        if (webSocketManager == null) {
            webSocketManager = WebSocketManager(serverUrl).apply {
                onMessageReceived = { message ->
                    _lastMessage.value = message
                    showMessageNotification("New Alert: " + message.replace("_"," ").replaceFirstChar { c -> c.uppercase() })
                }
                onConnectionChange = { connected ->
                    _isConnected.value = connected
                    val status = if (connected) "Connected" else "Reconnecting..."
                    updateNotification("IoT Server - $status")
                    Log.d("WSService", "Connection: $connected")
                }
                connect()
            }
        }

        return START_STICKY
    }

    @SuppressLint("ScheduleExactAlarm")
    override fun onTaskRemoved(rootIntent: Intent?) {
        super.onTaskRemoved(rootIntent)
        val restartIntent = Intent(applicationContext, WebSocketForegroundService::class.java)
        restartIntent.setPackage(packageName)

        val pendingIntent = PendingIntent.getForegroundService(
            applicationContext,
            0,
            restartIntent,
            PendingIntent.FLAG_IMMUTABLE or PendingIntent.FLAG_UPDATE_CURRENT
        )

        val alarmManager = getSystemService(Context.ALARM_SERVICE) as AlarmManager
        alarmManager.setExact(
            AlarmManager.ELAPSED_REALTIME_WAKEUP,
            SystemClock.elapsedRealtime() + 100,
            pendingIntent
        )
    }

    override fun onDestroy() {
        Log.d("WSService", "Service destroyed")

        webSocketManager?.disconnect()
        super.onDestroy()
    }

    private fun createServiceNotificationChannel() {
        val channel = NotificationChannel(
            CHANNEL_ID,
            "IoT WebSocket Service",
            NotificationManager.IMPORTANCE_DEFAULT
        ).apply {
            description = "Keeps WebSocket connection alive"
            setShowBadge(false)
        }
        notificationManager.createNotificationChannel(channel)
    }

    private fun createMessageNotificationChannel() {
        val channel = NotificationChannel(
            MESSAGE_CHANNEL_ID,
            "WebSocket Messages",
            NotificationManager.IMPORTANCE_HIGH
        ).apply {
            description = "Incoming WebSocket messages"
            enableVibration(true)
            enableLights(true)
        }
        notificationManager.createNotificationChannel(channel)
    }

    private fun showMessageNotification(message: String) {
        val intent = Intent(this, MainActivity::class.java).apply {
            flags = Intent.FLAG_ACTIVITY_NEW_TASK or Intent.FLAG_ACTIVITY_CLEAR_TASK
        }
        val pendingIntent = PendingIntent.getActivity(
            this, 0, intent,
            PendingIntent.FLAG_UPDATE_CURRENT or PendingIntent.FLAG_IMMUTABLE
        )

        val notification = NotificationCompat.Builder(this, MESSAGE_CHANNEL_ID)
            .setContentTitle("Alert")
            .setContentText(message)
            .setSmallIcon(R.drawable.ic_launcher_foreground)
            .setPriority(NotificationCompat.PRIORITY_HIGH)
            .setCategory(NotificationCompat.CATEGORY_MESSAGE)
            .setAutoCancel(true)
            .setContentIntent(pendingIntent)
            .build()

        notificationManager.notify(Random.nextInt(1000, 9999), notification)
    }

    private fun createNotification(contentText: String): Notification {
        val intent = Intent(this, MainActivity::class.java).apply {
            flags = Intent.FLAG_ACTIVITY_NEW_TASK or Intent.FLAG_ACTIVITY_CLEAR_TASK
        }
        val pendingIntent = PendingIntent.getActivity(
            this, 0, intent,
            PendingIntent.FLAG_UPDATE_CURRENT or PendingIntent.FLAG_IMMUTABLE
        )

        return NotificationCompat.Builder(this, CHANNEL_ID)
            .setContentTitle("IoT Control Is Active")
//            .setContentText(contentText)
            .setSmallIcon(R.drawable.ic_launcher_foreground)
            .setContentIntent(pendingIntent)
            .setOngoing(true)
            .setAutoCancel(false)
            .setCategory(NotificationCompat.CATEGORY_SERVICE)
            .setPriority(NotificationCompat.PRIORITY_LOW)
            .build()
    }

    private fun updateNotification(contentText: String) {
        try {
            val notification = createNotification(contentText)
            startForeground(
                NOTIFICATION_ID, notification,
                ServiceInfo.FOREGROUND_SERVICE_TYPE_DATA_SYNC
            )
        } catch (e: Exception) {
            Log.e("WSService", "Failed to update notification", e)
        }
    }

    companion object {
        private const val CHANNEL_ID = "iot_websocket_service"
        private const val MESSAGE_CHANNEL_ID = "notification_channel_id"
        private const val NOTIFICATION_ID = 1
    }
}