import './style.css';
import WizardAPI from './api.js';
import { injectUI } from './ui.js';

async function initIndex() {
    injectUI();
    const leaderboardPreview = document.getElementById('leaderboard-preview');
    if (!leaderboardPreview) return;

    try {
        const data = await WizardAPI.getLeaderboard(3);
        leaderboardPreview.innerHTML = data.players.map(player => `
            <tr>
                <td class="font-mono">${String(player.rank).padStart(2, '0')}</td>
                <td>${player.username}</td>
                <td class="font-mono">${player.wins}</td>
                <td class="font-mono">${player.losses || 0}</td>
                <td class="font-mono">${player.elo}</td>
            </tr>
        `).join('');
    } catch (error) {
        console.error('Failed to load leaderboard preview:', error);
    }
}

document.addEventListener('DOMContentLoaded', initIndex);
