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
        copyBtn.textContent = 'Copy';
        copyBtn.style.cssText = 'width: 100%; margin-top: 8px;';
        copyBtn.addEventListener('click', () => {
            navigator.clipboard.writeText(apiKey).then(() => {
                copyBtn.textContent = 'Copied!';
                setTimeout(() => { copyBtn.textContent = 'Copy'; }, 2000);
            });
        });
        apiKeyDisplay.parentNode.appendChild(copyBtn);
    }

    const profileSummary = document.getElementById('profile-summary');
    const matchesList = document.getElementById('matches-list');
    const statElo = document.getElementById('stat-elo');
    const statWins = document.getElementById('stat-wins');
    const statLosses = document.getElementById('stat-losses');

    try {
        const profile = await WizardAPI.getProfile();
        if (profileSummary) {
            const avatarHtml = profile.avatar_url
                ? `<img src="${profile.avatar_url}" style="width: 48px; height: 48px; object-fit: cover; border-radius: 50%; border: 2px solid var(--color-border);">`
                : `<div style="width: 48px; height: 48px; background: var(--color-surface-raised); border: 2px solid var(--color-border); border-radius: 50%; display: flex; align-items: center; justify-content: center; font-family: var(--font-display); font-weight: 700; color: var(--color-primary);">${profile.username.substring(0, 2).toUpperCase()}</div>`;

            profileSummary.innerHTML = `
                <div style="display: flex; align-items: center; gap: var(--spacing-md);">
                    ${avatarHtml}
                    <div>
                        <div style="font-weight: 700; font-size: 16px;">${profile.username}</div>
                        <div style="font-size: 12px; color: var(--color-text-dim);">Wizard</div>
                    </div>
                </div>
            `;
        }

        if (statElo) statElo.textContent = profile.elo;
        if (statWins) statWins.textContent = profile.wins;
        if (statLosses) statLosses.textContent = profile.losses || 0;
    } catch (error) {
        console.error('Failed to load profile:', error);
    }

    try {
        const { matches } = await WizardAPI.getMatches(5);
        if (matchesList) {
            matchesList.innerHTML = matches.map(match => `
                <li style="font-size: 13px; display: flex; justify-content: space-between; border-bottom: 1px solid var(--color-border); padding: 6px 0;">
                    <span>vs ${match.opponent}</span>
                    <span style="color: ${match.result === 'win' ? 'var(--color-secondary)' : 'var(--color-primary)'}; font-weight: 700;">${match.result === 'win' ? 'Win' : 'Loss'}</span>
                </li>
            `).join('') || '<li style="font-size: 13px; color: var(--color-text-dim);">No matches yet — go play!</li>';
        }
    } catch (error) {
        console.error('Failed to load matches:', error);
    }
}

document.addEventListener('DOMContentLoaded', initDashboard);
