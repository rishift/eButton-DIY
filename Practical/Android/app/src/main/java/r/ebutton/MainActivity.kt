package r.ebutton

import android.Manifest
import android.app.Activity
import android.content.BroadcastReceiver
import android.content.ContentValues.TAG
import android.content.Context
import android.content.Intent
import android.content.pm.PackageManager
import android.os.Bundle
import android.os.StrictMode
import android.telephony.PhoneStateListener
import android.telephony.SmsManager
import android.telephony.TelephonyManager
import android.util.Log
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.material3.Button
import androidx.compose.material3.Text
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.platform.LocalView
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import r.ebutton.ui.theme.AndroidTheme
import java.net.DatagramPacket
import java.net.DatagramSocket
import java.net.InetAddress
import java.util.Timer
import kotlin.concurrent.schedule


val socket = DatagramSocket(2222)
val address: InetAddress = InetAddress.getByName("192.168.4.1")
val receiveBuffer = ByteArray(120)
val packet = DatagramPacket(receiveBuffer, receiveBuffer.size)
var localContext: Context? = null;
var received = "";
val d = ByteArray(1)

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        socket.broadcast = false
        StrictMode.setThreadPolicy(StrictMode.ThreadPolicy.Builder().permitAll().build())

        UDPReceive().start()
        d[0] = 69;
        setContent {
            AndroidTheme(darkTheme = true) {
                LocalView.current.keepScreenOn = true
                localContext = LocalContext.current
                Button(onClick = { sendUDP(d) }) {
                    Text(text = "ok")
                }
            }
        }
    }
}

private fun sendUDP(data: ByteArray) {
    socket.send(DatagramPacket(data, 1, address, 1111))
}

class UDPReceive : Thread() {
    override fun run() {
        while (true) {
            socket.receive(packet)
            packet.address.hostAddress?.let { Log.d("Packet received from: ", it) }
            val data = String(packet.data).trim { it <= ' ' }
            Log.d("Packet received; data:", data)
            received = data;
            localContext?.let { sendMessage(it) }
        }
    }
}

fun sendMessage(context: Context) {
    try {
        val permission = Manifest.permission.SEND_SMS

        if (ContextCompat.checkSelfPermission(context, permission) == PackageManager.PERMISSION_GRANTED) {
            val smsManager: SmsManager = SmsManager.getDefault()
            smsManager.sendTextMessage("+918004237112", null, received, null, null)
            smsManager.sendTextMessage("+919909222351", null, received, null, null)
        } else {
            ActivityCompat.requestPermissions(context as Activity, arrayOf(permission), 0)
        }
    } catch (e: Exception) {
        e.printStackTrace()
    }
}

class CallReceiver : BroadcastReceiver() {
    override fun onReceive(context: Context?, p1: Intent?) {
        val telephonyManager: TelephonyManager = context?.getSystemService(Context.TELEPHONY_SERVICE) as TelephonyManager;
        telephonyManager.listen(object : PhoneStateListener() {
            override fun onCallStateChanged(state: Int, phoneNumber: String?) {
                super.onCallStateChanged(state, phoneNumber)
                when (state) {
                    TelephonyManager.CALL_STATE_IDLE -> {
                        Log.i(TAG, "not in call")
                    }

                    TelephonyManager.CALL_STATE_RINGING -> {
                        Log.i(TAG, "ringing")
                        sendUDP(d);
                        Timer().schedule(5000) {
                           localContext?.let { sendMessage(it) };
                        }
                    }

                }
            }
        }, PhoneStateListener.LISTEN_CALL_STATE);
    }
}

