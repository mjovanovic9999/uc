package com.example.iot_control.presentation

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import com.example.iot_control.domain.DHT11Data
import com.example.iot_control.domain.MQ2Data
import com.example.iot_control.presentation.cards.DHT11Card
import com.example.iot_control.presentation.cards.MQ2Card

@Composable
fun SensorDataList(dht11List: List<DHT11Data>, mq2List: List<MQ2Data>) {
    Row(
        modifier = Modifier
            .fillMaxSize()
            .padding(16.dp),
        horizontalArrangement = Arrangement.spacedBy(16.dp)
    ) {
        LazyColumn(
            modifier = Modifier.weight(1f),
            verticalArrangement = Arrangement.spacedBy(8.dp)
        ) {
            items(dht11List) { data ->
                DHT11Card(data)
            }
        }

        LazyColumn(
            modifier = Modifier.weight(1f),
            verticalArrangement = Arrangement.spacedBy(8.dp)
        ) {
            items(mq2List) { data ->
                MQ2Card(data)
            }
        }
    }
}

