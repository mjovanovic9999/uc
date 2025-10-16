package com.example.iot_control.presentation

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.height
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
import androidx.compose.runtime.DisposableEffect
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import androidx.lifecycle.viewmodel.compose.viewModel
import com.example.iot_control.presentation.cards.AlertCard

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun DashboardScreen() {
    val dashboardViewModel: DashboardViewModel = viewModel()
    val dht11List by dashboardViewModel.DHT11DataList.collectAsState()
    val mq2List by dashboardViewModel.MQ2DataList.collectAsState()
    val alert by dashboardViewModel.alerts.collectAsState()
    var showSettings by remember { mutableStateOf(false) }

    DisposableEffect(Unit) {
        onDispose {
            dashboardViewModel.disconnectMqtt()
        }
    }

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
                NotificationPermissionRequest()

                if (alert.isNotEmpty() && alert != "normal") {
                    AlertCard(alert)
                    Spacer(modifier = Modifier.height(8.dp))
                }
                SensorDataList(dht11List, mq2List)
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