rsync -avz /home/mj/Desktop/uc/III rpi@10.42.0.1:/home/rpi/Desktop/uc --exclude 'work/esp32/.pio' --exclude 'work/esp32/lib' --exclude 'work/IoT-Control'

mozda mi ni ne treba ovaj health endpoint

status:
- go
- stop
- forward

SELECTED ALERTS SAMO ZA WS kontejner

"esp32/cmd"
"esp32/status"
"esp32/cmd/blink"


mosquitto_pub -t 'sensors/dht11' -m '{"temperature":25,"humidity":40}' -r
mosquitto_pub -t 'sensors/mq2' -m '{"quality":40}' -r
mosquitto_pub -t 'alerts/all' -m '{"type": "fire"}' -r

mosquitto_pub -t 'alerts/select' -m 'gas_leak' -r

curl localhost:8080/health

## retained message za alerts sensors!!! (i status ako ide ceo status)


### android mqtt sub
```kotlin
implementation 'org.eclipse.paho:org.eclipse.paho.client.mqttv3:1.2.5'
implementation 'org.eclipse.paho:org.eclipse.paho.android.service:1.1.1'
//////////
import org.eclipse.paho.android.service.MqttAndroidClient
import org.eclipse.paho.client.mqttv3.*

val serverUri = "tcp://broker.hivemq.com:1883"
val clientId = MqttClient.generateClientId()
val topic = "test/topic"

val client = MqttAndroidClient(applicationContext, serverUri, clientId)

val options = MqttConnectOptions().apply {
    isCleanSession = true
}

client.connect(options, null, object : IMqttActionListener {
    override fun onSuccess(asyncActionToken: IMqttToken?) {
        // Connected
        client.subscribe(topic, 1)
    }

    override fun onFailure(asyncActionToken: IMqttToken?, exception: Throwable?) {
        // Failed to connect
    }
})

client.setCallback(object : MqttCallback {
    override fun messageArrived(topic: String?, message: MqttMessage?) {
        println("Message: ${message.toString()}")
    }

    override fun connectionLost(cause: Throwable?) {}
    override fun deliveryComplete(token: IMqttDeliveryToken?) {}
})


```

## fg service with notif
```kotlin
class WebSocketService : Service() {
    lateinit var webSocket: WebSocket

    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        startForeground(1, createNotification())
        connectWebSocket()
        return START_STICKY
    }

    private fun connectWebSocket() {
        val client = OkHttpClient()
        val request = Request.Builder().url("wss://yourserver.com/socket").build()
        webSocket = client.newWebSocket(request, object : WebSocketListener() {
            override fun onMessage(webSocket: WebSocket, text: String) {
                sendNotification(text)
            }
        })
    }

    private fun sendNotification(message: String) {
        val notificationManager = getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager
        val notification = NotificationCompat.Builder(this, "channel_id")
            .setContentTitle("New Message")
            .setContentText(message)
            .setSmallIcon(R.drawable.ic_notification)
            .build()
        notificationManager.notify(2, notification)
    }
}
```


sudo nmcli d wifi hotspot ifname wlan0 ssid test password test1234
nmcli dev wifi show-password
```
[Unit]  
Description=Auto Wi-Fi Hotspot  
After=network-online.target NetworkManager.service  
Wants=network-online.target  
  
[Service]  
Type=oneshot  
ExecStartPre=/bin/sleep 10  
ExecStart=/usr/bin/nmcli d wifi hotspot ifname wlan0 ssid test password test1234  
RemainAfterExit=yes  
  
[Install]  
WantedBy=multi-user.target
```

sudo iptables -t nat -A POSTROUTING -o wlan0 -j MASQUERADE  
sudo iptables -A FORWARD -i wlan0 -o eth0 -m state --state RELATED,ESTABLISHED -j ACCEPT  
sudo iptables -A FORWARD -i eth0 -o wlan0 -j ACCEPT

