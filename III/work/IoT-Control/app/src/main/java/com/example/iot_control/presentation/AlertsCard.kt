package com.example.iot_control.presentation

import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.Card
import androidx.compose.material3.CardDefaults
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp

@Composable
fun AlertsCard(alerts: List<String>) {
    Card(
        modifier = Modifier
            .fillMaxWidth()
            .padding(horizontal = 16.dp),
        colors = CardDefaults.cardColors(containerColor = Color(0xFFFF6F00))
    ) {
        Column(modifier = Modifier.padding(16.dp)) {
            Text(
                text = "Recent Alerts",
                fontWeight = FontWeight.Bold,
                color = Color.White,
                fontSize = 18.sp
            )
            Spacer(modifier = Modifier.height(8.dp))
            alerts.take(3).forEach { alert ->
                Text(text = "• $alert", color = Color.White, fontSize = 14.sp)
            }
        }
    }
}
