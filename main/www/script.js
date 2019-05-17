window.onload = function() {
	getInfo();
};

function getCurrentSSID() {
	sendRequest("getCurrentSSID", getCurrentSSIDHandler);
}
function getCurrentSSIDHandler(data) {
	document.getElementById("curSsid").value = data;
}

function getMAC() {
	sendRequest("getMAC", getMACHandler);
}
function getMACHandler(data) {
	document.getElementById("macAddr").value = data;
}

function getInfo() {
	sendRequest("getInfo", getInfoHandler);
}
function getInfoHandler(data) {
	var d = data.split(',');
	document.getElementById("curSsid").value = d[0];
	document.getElementById("macAddr").value = d[1];
}

function getSSIDList() {
	openPopup("Getting network names...", 'i');
	sendRequest("getSSIDList", getSSIDListHandler);
}
function getSSIDListHandler(data) {
	popup("Done", 's');
	if (data == null)
		return;

	var ssids = data.split(',');
	var list = document.getElementById("ssid");
	list.innerHTML = "";
	if (ssids.length == 0) {
		list.innerHTML = "<option value='0'>-</option>";
		return;
	}
	for (var i = 0; i < ssids.length; i++) {
		if (ssids[i].trim() == "")
			continue;

		var inList = false;
		for (var j = 0; j < i; j++) {
			if (ssids[i] == ssids[j]) {
				console.log("SSID already in list");
				inList = true;
			}
		}
		
		if (!inList)
			list.innerHTML += "<option value='" + ssids[i] + "'>" + ssids[i] + "</option>";
	}
}

function connect() {
	var ssid = document.getElementById("ssid").value;
	var pass = document.getElementById("pass").value;
	if (ssid != "0")
		sendRequest("connect?ssid=" + ssid + "&pass=" + pass, connectHandler);
	else
		popup("No SSID selected");
}
function connectHandler(data) {
	openPopup(data, 'i');
	getWifiStateInterval = setInterval(getWifiState, 2000); //2 seconds
}

var getWifiStateInterval;
function getWifiState() {
	sendRequest("getWifiState", getWifiStateHandler);
}
function getWifiStateHandler(data) {
	var type = data.charAt(0);
	data = data.substr(1);

	if (type == 'e' || type == 's') {
		clearInterval(getWifiStateInterval);
		popup(data, type);
		getCurrentSSID();
	}
	else
		openPopup(data, type);
}

function sendRequest(addr, func=null) {
	xhttp = new XMLHttpRequest();
	xhttp.onload = function() {
		console.log("Received response: " + xhttp.response);
		if (func != null) {
			func(xhttp.response);
		}
	};
	xhttp.onerror = function() {
		clearInterval(getWifiStateInterval);
		popup("Can't reach host");
	};
	xhttp.open("GET", addr, true);
	console.log("Sending request: " + addr);
	xhttp.send();
}

var popupTimeout;
//popup types: e(rror), i(nfo), s(uccess)
function popup(text, type='e', duration=2500) {
	clearTimeout(popupTimeout);
	openPopup(text, type);
	popupTimeout = setTimeout(closePopup, duration);
}
function openPopup(text, type='e') {
	var div = document.getElementById("popup");
	div.className = "popup";
	div.innerHTML = text;
	if (type == 'e')
		div.classList.add("error");
	else if (type == 'i')
		div.classList.add("info");
	else if (type == 's')
		div.classList.add("success");
	div.classList.add("open");
}
function closePopup() {
	var div = document.getElementById("popup");
	div.className = "popup";
}