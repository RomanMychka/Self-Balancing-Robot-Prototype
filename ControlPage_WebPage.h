#pragma once

const char* controlPageHTML = R"rawHTML(
<!DOCTYPE html>
<html lang="uk">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
    <title>Balance Bot Controller</title>
    <style>
        :root {
            --primary-color: #263238;
            --bg-color: #eceff1;
            --btn-bg: #ffffff;
            --btn-active: #6ad975; /* Колір при натисканні */
            --icon-color: #546e7a;
        }

        body {
            font-family: sans-serif;
            background-color: var(--bg-color);
            margin: 0;
            display: flex;
            flex-direction: column;
            height: 100vh;
            user-select: none;
            -webkit-user-select: none;
            touch-action: none; /* Вимикає стандартні жести браузера */
        }

        header {
            background-color: var(--primary-color);
            color: white;
            padding: 15px;
            display: flex;
            justify-content: space-between;
        }

        .gamepad-container {
            display: flex;
            flex: 1;
            justify-content: space-around;
            align-items: center;
            padding: 20px;
        }

        .btn {
            width: 80px;
            height: 80px;
            background: var(--btn-bg);
            border: none;
            border-radius: 20px;
            display: flex;
            justify-content: center;
            align-items: center;
            transition: 0.1s;
        }

        /* Клас, який ми будемо додавати через JS при натисканні */
        .btn.active {
            background-color: var(--btn-active);
            transform: scale(0.95);
        }

        .arrow-icon {
            width: 40px;
            height: 40px;
            fill: var(--icon-color);
        }

        .rot-up { transform: rotate(-90deg); }
        .rot-down { transform: rotate(90deg); }
        .rot-left { transform: rotate(180deg); }

        .sliders { display: flex; flex-direction: column; gap: 20px; }
        input[type=range] { height: 150px; writing-mode: bt-lr; appearance: slider-vertical; }
    </style>
</head>
<body>

<header>
    <div>BALANCE BOT</div>
    <div onclick="setIP()"><span id="ipDisplay">...</span> ✎</div>
</header>

<div class="gamepad-container">
    <div class="drive-group">
        <button class="btn" id="btn-f"><svg class="arrow-icon rot-up" viewBox="0 0 100 100"><polygon points="25,25 75,50 25,75"/></svg></button>
        <div style="height: 10px;"></div>
        <button class="btn" id="btn-b"><svg class="arrow-icon rot-down" viewBox="0 0 100 100"><polygon points="25,25 75,50 25,75"/></svg></button>
    </div>

    <div class="sliders">
        <input type="range" id="speedRange" min="0" max="255" value="150">
        <label>SPEED</label>
    </div>

    <div class="turn-group" style="display: flex; gap: 10px;">
        <button class="btn" id="btn-l"><svg class="arrow-icon rot-left" viewBox="0 0 100 100"><polygon points="25,25 75,50 25,75"/></svg></button>
        <button class="btn" id="btn-r"><svg class="arrow-icon" viewBox="0 0 100 100"><polygon points="25,25 75,50 25,75"/></svg></button>
    </div>
</div>

<script>
    let robotIP = localStorage.getItem('robotIP') || window.location.hostname;
    document.getElementById('ipDisplay').innerText = robotIP;

    // Словник станів: які кнопки зараз натиснуті
    const inputs = { f: 0, b: 0, l: 0, r: 0 };
    let lastQuery = "";

    // Налаштування обробників для кнопок
    setupBtn("btn-f", 'f');
    setupBtn("btn-b", 'b');
    setupBtn("btn-l", 'l');
    setupBtn("btn-r", 'r');

    function setupBtn(id, key) {
        const el = document.getElementById(id);
        const start = (e) => { e.preventDefault(); inputs[key] = 1; el.classList.add('active'); };
        const end = (e) => { e.preventDefault(); inputs[key] = 0; el.classList.remove('active'); };
        
        el.addEventListener('pointerdown', start);
        el.addEventListener('pointerup', end);
        el.addEventListener('pointerleave', end);
    }

    // Головний таймер: кожні 50мс збирає стан і відправляє на ESP
    setInterval(() => {
        const v = inputs.f - inputs.b; // 1 (вперед), -1 (назад), 0 (стоп)
        const h = inputs.r - inputs.l; // 1 (вправо), -1 (вліво), 0 (прямо)
        const s = document.getElementById('speedRange').value;

        const currentQuery = `move?v=${v}&h=${h}&s=${s}`;
        
        // Відправляємо запит ТІЛЬКИ якщо стан змінився
        if (currentQuery !== lastQuery) {
            fetch(`http://${robotIP}/${currentQuery}`).catch(()=>{});
            lastQuery = currentQuery;
        }
    }, 50);

    function setIP() {
        let res = prompt("IP адреса:", robotIP);
        if(res) { robotIP = res; localStorage.setItem('robotIP', res); location.reload(); }
    }
</script>
</body>
</html>
)rawHTML";