package com.example.iot_control.core

import android.app.NotificationManager
import android.content.Context
import androidx.core.app.NotificationCompat
import com.example.iot_control.R
import kotlin.random.Random

class NotificationHandler(private val context: Context) {
    private val notificationManager = context.getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager
    private val notificationChannelID = "notification_channel_id" // Must match channel ID

    fun showSimpleNotification(title: String, message: String) {
        // Build the notification
        val notification = NotificationCompat.Builder(context, notificationChannelID)
            .setContentTitle(title)
            .setContentText(message)
            .setSmallIcon(R.drawable.ic_launcher_foreground)
            .setPriority(NotificationCompat.PRIORITY_HIGH)
            .setAutoCancel(true) // Dismisses when tapped
            .build()

        // Show the notification with a unique ID
        notificationManager.notify(Random.nextInt(), notification)
    }
}