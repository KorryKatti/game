import './style.css';
import WizardAPI from './api.js';
import { injectUI } from './ui.js';

async function initLeaderboard() {
    injectUI();
    const tbody = document.getElementById('leaderboard-body');
    if (!tbody) return;

    try {
        const data = await WizardAPI.getLeaderboard(50);
        tbody.innerHTML = data.players.map(player => `
            <tr>
                <td class="font-mono">${String(player.rank).padStart(2, '0')}</td>
                <td>${player.username}</td>
                <td class="font-mono">${player.wins}</td>
                <td class="font-mono">${player.losses || 0}</td>
                <td class="font-mono">${player.elo}</td>
                <td>${player.region || 'GLOBAL'}</td>
            </tr>
        `).join('');
    } catch (error) {
        console.error('Failed to load leaderboard:', error);
    }
}

document.addEventListener('DOMContentLoaded', initLeaderboard);
