async function loadStatus() {
    document.getElementById('status-text').textContent = 'Pinging backend...';
    try {
        const response = await fetch('/api/status');
        const data = await response.json();
        const { engine, status } = data;
        const time = new Date().toLocaleTimeString();
        document.getElementById('status-text').textContent = data.message;
        document.getElementById('stat-engine').textContent = engine;
        document.getElementById('stat-status').textContent = status.toUpperCase();
        document.getElementById('stat-time').textContent = time;
    } catch (e) {
        document.getElementById('status-text').textContent = 'Backend unreachable';
    }
}

async function sendName() {
    const name = document.getElementById('nameInput').value.trim();
    if (!name) return;
    const replyBox = document.getElementById('reply');
    const replyText = document.getElementById('reply-text');
    replyBox.classList.add('hidden');
    try {
        const response = await fetch('/api/greet?name=' + encodeURIComponent(name));
        const data = await response.json();
        const { reply } = data;
        replyText.textContent = reply;
        replyBox.classList.remove('hidden');
    } catch (e) {
        replyText.textContent = 'Could not reach backend.';
        replyBox.classList.remove('hidden');
    }
}

function scrollToStatus() {
    document.getElementById('status-section').scrollIntoView({ behavior: 'smooth'});
}

function scrollToInteract() {
    document.getElementById('interact-section').scrollIntoView({ behavior: 'smooth' });
}

// Allow Enter key to send
document.addEventListener('DOMContentLoaded', () => {
    loadStatus();
    document.getElementById('nameInput').addEventListener('keydown', async (e) => {
        if (e.key === 'Enter') await sendName();
    });
});