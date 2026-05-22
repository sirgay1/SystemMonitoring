// Объявляем глобальный объект Chart из CDN (без импорта)
declare const Chart: any;

// Интерфейсы для данных
interface ProcessInfo {
    pid: number;
    name: string;
    cpu: number;
    mem: number;
}

interface SystemStats {
    cpu_usage: number;
    memory: {
        total_mb: number;
        available_mb: number;
        percent: number;
    };
    processes: ProcessInfo[];
}

let cpuChart: any = null;

// Запрос статистики с сервера
async function fetchStats(): Promise<void> {
    try {
        const response = await fetch('/api/stats');
        if (!response.ok) throw new Error(`HTTP ${response.status}`);
        const data: SystemStats = await response.json();
        updateUI(data);
    } catch (err) {
        console.error('Failed to fetch stats:', err);
    }
}

// Обновление интерфейса
function updateUI(data: SystemStats): void {
    // CPU
    const cpuUsage = data.cpu_usage;
    const cpuValueElem = document.getElementById('cpuValue');
    if (cpuValueElem) cpuValueElem.innerText = cpuUsage.toFixed(1) + '%';

    // График CPU
    if (cpuChart) {
        const dataset = cpuChart.data.datasets[0].data as number[];
        dataset.push(cpuUsage);
        if (dataset.length > 60) dataset.shift();
        cpuChart.update();
    }

    // Память
    const memPercent = data.memory.percent;
    const memFill = document.getElementById('memFill');
    const memValueElem = document.getElementById('memValue');
    const memDetailsElem = document.getElementById('memDetails');
    if (memFill) (memFill as HTMLElement).style.width = memPercent + '%';
    if (memValueElem) memValueElem.innerText = memPercent.toFixed(1) + '%';
    if (memDetailsElem) {
        const total = data.memory.total_mb;
        const available = data.memory.available_mb;
        const used = total - available;
        memDetailsElem.innerText = `${used.toFixed(0)} MB / ${total.toFixed(0)} MB used`;
    }

    // Таблица процессов
    const tbody = document.querySelector('#processTable tbody');
    if (tbody && tbody instanceof HTMLTableSectionElement) {
        tbody.innerHTML = '';
        const topProcesses = data.processes.slice(0, 50);
        for (const proc of topProcesses) {
            const row = tbody.insertRow();
            row.insertCell(0).innerText = proc.pid.toString();
            row.insertCell(1).innerText = proc.name;
            row.insertCell(2).innerText = proc.cpu.toFixed(1);
            row.insertCell(3).innerText = proc.mem.toFixed(1);
        }
    }
}

// Инициализация графика
function initChart(): void {
    const canvas = document.getElementById('cpuChart') as HTMLCanvasElement;
    if (!canvas) return;
    const ctx = canvas.getContext('2d');
    if (!ctx) return;
    cpuChart = new Chart(ctx, {
        type: 'line',
        data: {
            labels: Array(60).fill(''),
            datasets: [{
                label: 'CPU %',
                data: Array(60).fill(0),
                borderColor: 'rgb(255, 204, 0)',
                backgroundColor: 'rgba(255, 204, 0, 0.1)',
                tension: 0.3,
                fill: true,
                pointRadius: 0
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: true,
            scales: {
                y: { beginAtZero: true, max: 100, title: { display: true, text: '%' } },
                x: { ticks: { display: false } }
            },
            plugins: { legend: { display: false } }
        }
    });
}

// Запуск после загрузки DOM
document.addEventListener('DOMContentLoaded', () => {
    initChart();
    fetchStats();
    setInterval(fetchStats, 1000);
});
