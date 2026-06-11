import './style.css';
import WizardAPI from './api.js';
import { injectUI } from './ui.js';

async function initDashboard() {
    injectUI();
    
    const apiKey = WizardAPI.getApiKey();
    if (!apiKey) {
        window.location.href = '/login.html';
        return;
    }

    const apiKeyDisplay = document.getElementById('api-key-display');
    if (apiKeyDisplay) {
        apiKeyDisplay.textContent = apiKey;
        const copyBtn = document.createElement('button');
        copyBtn.className = 'btn btn-sm';
        copyBtn.textContent = 'COPY';
        copyBtn.style.cssText = 'width: 100%; margin-top: 4px; font-size: 9px; padding: 4px;';
        copyBtn.addEventListener('click', () => {
            navigator.clipboard.writeText(apiKey).then(() => {
                copyBtn.textContent = 'COPIED';
                setTimeout(() => { copyBtn.textContent = 'COPY'; }, 2000);
            });
        });
        apiKeyDisplay.parentNode.appendChild(copyBtn);
    }

    const profileSummary = document.getElementById('profile-summary');
    const matchesList = document.getElementById('matches-list');
    const onlineList = document.getElementById('online-list');

    // Load Profile
    try {
        const profile = await WizardAPI.getProfile();
        if (profileSummary) {
            const avatarHtml = profile.avatar_url
                ? `<img src="${profile.avatar_url}" style="width: 48px; height: 48px; object-fit: cover; border: 1px solid var(--color-border);">`
                : `<div style="width: 48px; height: 48px; background: var(--color-surface-raised); border: 1px solid var(--color-border); display: flex; align-items: center; justify-content: center; font-family: var(--font-display); font-weight: 700;">${profile.username.substring(0, 2).toUpperCase()}</div>`;

            profileSummary.innerHTML = `
                <div style="display: flex; align-items: center; gap: var(--spacing-md); margin-bottom: var(--spacing-md);">
                    ${avatarHtml}
                    <div>
                        <div style="font-weight: 700;">${profile.username.toUpperCase()}</div>
                        <div style="font-size: 12px; opacity: 0.7;">RANKED DUELIST</div>
                    </div>
                </div>
                <div style="display: grid; grid-template-columns: 1fr 1fr; gap: var(--spacing-md);">
                    <div>
                        <div class="stat-label">ELO</div>
                        <div class="stat-value">${profile.elo}</div>
                    </div>
                    <div>
                        <div class="stat-label">WINS</div>
                        <div class="stat-value">${profile.wins}</div>
                    </div>
                </div>
                <button id="logout-btn" class="btn" style="width: 100%; margin-top: var(--spacing-md); font-size: 10px; padding: 4px;">LOGOUT</button>
            `;
            document.getElementById('logout-btn').addEventListener('click', () => {
                WizardAPI.clearApiKey();
                window.location.href = '/';
            });
        }
    } catch (error) {
        console.error('Failed to load profile:', error);
    }

    // Load Matches
    try {
        const { matches } = await WizardAPI.getMatches(5);
        if (matchesList) {
            matchesList.innerHTML = matches.map(match => `
                <li style="font-size: 12px; display: flex; justify-content: space-between; border-bottom: 1px solid var(--color-border); padding-bottom: 4px;">
                    <span>vs ${match.opponent.toUpperCase()}</span>
                    <span style="color: ${match.result === 'win' ? 'var(--color-teal)' : 'var(--color-crimson)'}; font-weight: 700;">${match.result.toUpperCase()}</span>
                </li>
            `).join('') || '<li style="font-size: 12px; opacity: 0.5;">No matches played</li>';
        }
    } catch (error) {
        console.error('Failed to load matches:', error);
    }

    // Load Online Players
    try {
        const { players } = await WizardAPI.getOnlinePlayers();
        if (onlineList) {
            onlineList.innerHTML = players.map(player => `
                <li style="display: flex; justify-content: space-between; align-items: center;">
                    <div style="display: flex; align-items: center;">
                        <span class="online-badge"></span>
                        <span>${player.username.toUpperCase()}</span>
                    </div>
                    <button class="btn challenge-btn" data-username="${player.username}" style="padding: 4px 8px; font-size: 10px;">CHALLENGE</button>
                </li>
            `).join('') || '<li style="font-size: 12px; opacity: 0.5;">No players online</li>';

            document.querySelectorAll('.challenge-btn').forEach(btn => {
                btn.addEventListener('click', (e) => {
                    alert(`Challenge sent to ${e.target.dataset.username}`);
                });
            });
        }
    } catch (error) {
        console.error('Failed to load online players:', error);
    }

    // Mock Actions
    document.querySelectorAll('.action-bar .btn').forEach(btn => {
        btn.addEventListener('click', () => {
            const logPanel = document.getElementById('match-log');
            const action = btn.textContent;
            const time = new Date().toLocaleTimeString([], { minute: '2-digit', second: '2-digit' });
            const log = document.createElement('div');
            log.textContent = `[${time}] ${action} executed`;
            logPanel.prepend(log);
        });
    });
}

document.addEventListener('DOMContentLoaded', initDashboard);
