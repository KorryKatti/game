import './style.css';
import WizardAPI from './api.js';
import { injectUI } from './ui.js';

async function initProfile() {
    injectUI();
    if (!WizardAPI.getApiKey()) {
        window.location.href = '/login.html';
        return;
    }

    try {
        const profile = await WizardAPI.getProfile();
        const avatarPlaceholder = document.getElementById('profile-avatar-placeholder');
        if (avatarPlaceholder) {
            if (profile.avatar_url) {
                avatarPlaceholder.innerHTML = `<img src="${profile.avatar_url}" style="width: 100px; height: 100px; object-fit: cover;">`;
            } else {
                avatarPlaceholder.textContent = profile.username.substring(0, 2).toUpperCase();
            }
        }
        document.getElementById('profile-username').textContent = profile.username.toUpperCase();
        document.getElementById('profile-elo').textContent = profile.elo;
        document.getElementById('profile-wins').textContent = profile.wins;
        document.getElementById('profile-losses').textContent = profile.losses;
        document.getElementById('profile-created').textContent = new Date(profile.created_at).toLocaleDateString();
    } catch (error) {
        console.error('Failed to load profile:', error);
    }
}

document.addEventListener('DOMContentLoaded', initProfile);
