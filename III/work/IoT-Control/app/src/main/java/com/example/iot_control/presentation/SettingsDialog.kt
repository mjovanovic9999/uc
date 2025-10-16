package com.example.iot_control.presentation

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.Button
import androidx.compose.material3.Card
import androidx.compose.material3.Checkbox
import androidx.compose.material3.FilterChip
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Switch
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.compose.ui.window.Dialog

@Composable
fun SettingsDialog(
    dashboardViewModel: DashboardViewModel, onDismiss: () -> Unit
) {
    val mcuState by dashboardViewModel.mcuState.collectAsState()
    val measurePeriod by dashboardViewModel.measurePeriod.collectAsState()
    val subscribedAlerts by dashboardViewModel.subscribedAlerts.collectAsState()
    val notificationsEnabled by dashboardViewModel.notificationsEnabled.collectAsState()

    var periodInput by remember { mutableStateOf(measurePeriod.toString()) }

    Dialog(onDismissRequest = onDismiss) {
        Card(
            modifier = Modifier
                .fillMaxWidth()
                .padding(5.dp), shape = RoundedCornerShape(16.dp)
        ) {
            Column(
                modifier = Modifier
                    .padding(16.dp)
                    .verticalScroll(rememberScrollState())
            ) {
                Text(
                    text = "Settings", fontSize = 24.sp, fontWeight = FontWeight.Bold
                )

                Spacer(modifier = Modifier.height(24.dp))

                Text(
                    text = "MCU State", fontSize = 16.sp, fontWeight = FontWeight.SemiBold
                )
                Spacer(modifier = Modifier.height(8.dp))
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.spacedBy(4.dp)
                ) {
                    listOf("standby", "measure", "forward").forEach { state ->
                        FilterChip(
                            selected = mcuState == state,
                            onClick = { dashboardViewModel.setMcuState(state) },
                            label = { Text(state.replaceFirstChar { it.uppercase()}) },
                            modifier = Modifier.weight(1f)
                        )
                    }
                }

                Spacer(modifier = Modifier.height(24.dp))

                Text(
                    text = "Measure Period (seconds)",
                    fontSize = 16.sp,
                    fontWeight = FontWeight.SemiBold
                )
                Spacer(modifier = Modifier.height(8.dp))
                OutlinedTextField(
                    value = periodInput,
                    onValueChange = {
                        periodInput = it
                        it.toIntOrNull()?.let { period ->
                            if (period > 0) dashboardViewModel.setMeasurePeriod(period)
                        }
                    },
                    modifier = Modifier.fillMaxWidth(),
                    keyboardOptions = KeyboardOptions(keyboardType = KeyboardType.Number),
                    singleLine = true
                )

                Spacer(modifier = Modifier.height(24.dp))
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    verticalAlignment = Alignment.CenterVertically,
                    horizontalArrangement = Arrangement.SpaceBetween
                ) {
                    Text(
                        text = "Alert Notifications",
                        fontSize = 16.sp,
                        fontWeight = FontWeight.SemiBold
                    )
                    Switch(
                        checked = notificationsEnabled,
                        onCheckedChange = { dashboardViewModel.toggleNotifications(it) }
                    )
                }
                Spacer(modifier = Modifier.height(16.dp))

                Text(
                    text = "Alert Subscriptions", fontSize = 16.sp, fontWeight = FontWeight.SemiBold
                )
                Spacer(modifier = Modifier.height(8.dp))

                listOf("normal", "high_humidity", "gas_leak", "fire").forEach { alert ->
                    Row(
                        modifier = Modifier
                            .fillMaxWidth(),
                        verticalAlignment = Alignment.CenterVertically
                    ) {
                        Checkbox(
                            checked = subscribedAlerts.contains(alert), onCheckedChange = {
                                dashboardViewModel.toggleAlertSubscription(alert)
                            })
                        Spacer(modifier = Modifier.width(2.dp))
                        Text(
                            text = alert.replace("_", " ").replaceFirstChar { it.uppercase() },
                            fontSize = 14.sp
                        )
                    }
                }
                Spacer(modifier = Modifier.height(16.dp))
                Button(
                    onClick = { dashboardViewModel.blinkLed() }, modifier = Modifier.fillMaxWidth()
                ) {
                    Spacer(modifier = Modifier.width(8.dp))
                    Text("Blink RGB LED")
                }

                // Close Button
                TextButton(
                    onClick = onDismiss, modifier = Modifier.align(Alignment.End)
                ) {
                    Text("Close")
                }
            }
        }
    }
}