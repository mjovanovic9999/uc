package com.example.iot_control.presentation

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Settings
import androidx.compose.material3.Button
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Text
import androidx.compose.material3.TopAppBar
import androidx.compose.material3.TopAppBarDefaults
import androidx.compose.material3.darkColorScheme
import androidx.compose.runtime.Composable
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.platform.LocalContext
import androidx.lifecycle.viewmodel.compose.viewModel

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun DashboardScreen() {
    val dashboardViewModel: DashboardViewModel = viewModel()
    val sensorDataList by dashboardViewModel.sensorDataList.collectAsState()
    val isConnected by dashboardViewModel.isConnected.collectAsState()
    val alerts by dashboardViewModel.alerts.collectAsState()
    var showSettings by remember { mutableStateOf(false) }

    val context = LocalContext.current

    MaterialTheme(colorScheme = darkColorScheme()) {
        Scaffold(
            topBar = {
                TopAppBar(
                    title = { Text("IoT Dashboard") },
                    actions = {
                        IconButton(onClick = { showSettings = true }) {
                            Icon(Icons.Default.Settings, "Settings")
                        }
                    },
                    colors = TopAppBarDefaults.topAppBarColors(
                        containerColor = MaterialTheme.colorScheme.primaryContainer
                    )
                )
            }
        ) { padding ->
            Column(
                modifier = Modifier
                    .fillMaxSize()
                    .padding(padding)
                    .background(MaterialTheme.colorScheme.background)
            ) {
                ConnectionCard(isConnected, dashboardViewModel)
                if (alerts.isNotEmpty()) {
                    AlertsCard(alerts)
                }

                Row {
                    Button(
                        onClick = {
                            dashboardViewModel.connectInApp()
                            // Show message that service will continue in background
                        },
                        enabled = !isConnected
                    ) {
                        Text("Connect (App)")
                    }

                    Button(
                        onClick = {
                            dashboardViewModel.connectInBackground()
                        },
                        enabled = !isConnected
                    ) {
                        Text("Connect (Background)")
                    }

                    Button(
                        onClick = { dashboardViewModel.disconnect() },
                        enabled = isConnected
                    ) {
                        Text("Disconnect All")
                    }
                }

                // Connection status with service info
                Text(
                    text = if (isConnected) "Connected - Running in background" else "Disconnected",
                    color = if (isConnected) Color.Green else Color.Red
                )
                RequestPermission()
                SensorDataList(sensorDataList)

            }

            if (showSettings) {
                SettingsDialog(
                    dashboardViewModel = dashboardViewModel,
                    onDismiss = { showSettings = false }
                )
            }

        }
    }
}