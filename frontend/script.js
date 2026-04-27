async function loadStatus() {
    document.getElementById('status').innerText = 'Checking backend...';

    const response = await fetch('/api/status');
    const data = await response.json();

    const time = new Date().toLocaleTimeString();

    document.getElementById('status').innerText =
        data.status + ' | Engine: ' + data.engine + ' | ' + data.message + ' | Checked at ' + time;
}

async function sendName() {
    const name = document.getElementById('nameInput').value;

    document.getElementById('reply').innerText = 'Sending to C++ Backend...';

    const response = await fetch('/api/greet?name=' + encodeURIComponent(name));
    const data = await response.json();

    document.getElementById('reply').innerText = data.reply;
}

window.onload = loadStatus;