/*
   Eclipse Paho MQTT-JS Utility
   by Elliot Williams for Hackaday article, 
*/

// Global variables
var client       = null;
var led_is_on    = null;
// These are configs	
//var hostname       = "192.168.2.254";
//var port           = "9001";
var clientId       = "mqtt_js_" + parseInt(Math.random() * 100000, 10);
var temp_topic     = "temperature1";
var humidity_topic = "home/outdoors/humidity";
var status_topic   = "brightness";



MQTTport
// This is called after the webpage is completely loaded
// It is the main entry point into the JS code
function connect(){
	// Set up the client
	client = new Paho.MQTT.Client(MQTTbroker, Number(MQTTport), clientId);
	console.info('Connecting to Server: Hostname: ', MQTTbroker, 
			'. Port: ', MQTTport, '. Client ID: ', clientId);

	// set callback handlers
	client.onConnectionLost = onConnectionLost;
	client.onMessageArrived = onMessageArrived;

	// see client class docs for all the options
	var options = {
		onSuccess: onConnect, // after connected, subscribes
		onFailure: onFail     // useful for logging / debugging
	};
	// connect the client
	client.connect(options);
	console.info('Connecting...');
}


function onConnect(context) {
	console.log("Client Connected");
    // And subscribe to our topics	-- both with the same callback function
	options = {qos:0, onSuccess:function(context){ console.log("subscribed"); } }
	client.subscribe(temp_topic, options);
	client.subscribe(humidity_topic, options);
	client.subscribe(status_topic, options);
}

function onFail(context) {
	console.log("Failed to connect");
}

function onConnectionLost(responseObject) {
	if (responseObject.errorCode !== 0) {
		console.log("Connection Lost: " + responseObject.errorMessage);
		window.alert("Someone else took my websocket!\nRefresh to take it back.");
	}
}

// Here are the two main actions that drive the webpage:
//  handling incoming messages and the toggle button.

// Updates the webpage elements with new data, and 
//  tracks the display LED status as well, 
//  in case multiple clients are toggling it.
function onMessageArrived(message) {
	console.log(message.destinationName, message.payloadString);

	// Update element depending on which topic's data came in
	if (message.destinationName == temp_topic){ 
		var temperature_heading = document.getElementById("temp_display");
		temperature_heading.innerHTML = "Temperature: " + message.payloadString + " &deg;C";
	} else if (message.destinationName == humidity_topic) {
		var humidity_heading = document.getElementById("humidity_display");
		humidity_heading.innerHTML = "Humidity: " + message.payloadString + "%";
	} else if (message.destinationName == status_topic) {
		var status_heading = document.getElementById("led_status");
		status_heading.innerHTML = "LED Backlight Power (0-9)  : " + message.payloadString;
		// Accomodates one or two byte "on" commands.  Anything else is off.
		if (message.payloadString == "on" || message.payloadString == "o"){
			//led_is_on = true;
		} else {
			//led_is_on = false;
		}
	}
}

// Provides the button logic that toggles our display LED on and off
// Triggered by pressing the HTML button "status_button"
function led_toggle(){
	if (led_is_on){
		var payload = "0";
		led_is_on = false;
	} else {
		var payload = "9";
		led_is_on = true;

	}
      console.log("led_is_on",led_is_on);
	// Send messgae
	message = new Paho.MQTT.Message(payload);
	message.destinationName = status_topic;
	message.retained = true;
	client.send(message);
	console.info('sending: ', payload);
	}
	
	
	function led_power0(){
		sendMsg("0");
		}
	function led_power1(){
		sendMsg("1");
		}
	function led_power2(){
		sendMsg("2");
		}
	function led_power4(){
		sendMsg("4");
		}
	function led_power6(){
		sendMsg("6");
		}		
	function led_power8(){
		sendMsg("8");
		}
	function led_power9(){
		sendMsg("9");
		}
		
		
		
	// Send messgae
	function sendMsg(payload){
	message = new Paho.MQTT.Message(payload);
	message.destinationName = status_topic;
	message.retained = true;
	client.send(message);
	console.info('sending: ', payload);
	}


