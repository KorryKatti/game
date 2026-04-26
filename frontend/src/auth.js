import './style.css';
import WizardAPI from './api.js';
import { injectUI } from './ui.js';

const loginForm = document.getElementById('login-form');
const registerForm = document.getElementById('register-form');
const errorMessage = document.getElementById('error-message');

document.addEventListener('DOMContentLoaded', () => {
    injectUI();
});

const showError = (message) => {
    if (errorMessage) {
        errorMessage.textContent = message;
        errorMessage.style.display = 'block';
    } else {
        alert(message);
    }
};

if (loginForm) {
    loginForm.addEventListener('submit', async (e) => {
        e.preventDefault();
        try {
            const data = await WizardAPI.login(loginForm.username.value, loginForm.password.value);
            if (data.success) {
                WizardAPI.setApiKey(data.api_key);
                window.location.href = '/dashboard.html';
            }
        } catch (error) {
            showError(error.message);
        }
    });
}

if (registerForm) {
    registerForm.addEventListener('submit', async (e) => {
        e.preventDefault();
        try {
            const data = await WizardAPI.register(registerForm.username.value, registerForm.password.value, registerForm.avatar.value);
            if (data.success) {
                WizardAPI.setApiKey(data.api_key);
                window.location.href = '/dashboard.html';
            }
        } catch (error) {
            showError(error.message);
        }
    });
}
