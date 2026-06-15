import WizardAPI from './api.js';

export function injectUI() {
    const navPlaceholder = document.getElementById('nav-placeholder');
    const footerPlaceholder = document.getElementById('footer-placeholder');

    const currentPath = window.location.pathname;
    const isLoggedIn = !!WizardAPI.getApiKey();

    if (navPlaceholder) {
        navPlaceholder.innerHTML = `
            <nav>
                <div class="container nav-content">
                    <a href="/" id="logo">Wizard Duel</a>
                    <div class="nav-links">
                        <a href="/leaderboard.html" class="${currentPath.includes('leaderboard') ? 'active' : ''}">Ranks</a>
                        <a href="/docs.html" class="${currentPath.includes('docs') ? 'active' : ''}">Spells</a>
                        ${isLoggedIn
                            ? `<a href="/dashboard.html" class="${currentPath.includes('dashboard') ? 'active' : ''}">Dashboard</a>`
                            : `<a href="/login.html">Login</a>`}
                    </div>
                </div>
            </nav>
        `;
    }

    if (footerPlaceholder) {
        footerPlaceholder.innerHTML = `
            <footer style="border-top: 1px solid #222; padding: 24px 0; margin-top: 40px;">
                <div class="container">
                    <p class="muted" style="font-size: 12px;">Made by <a href="https://korrykatti.github.io" target="_blank">korry</a></p>
                </div>
            </footer>
        `;
    }
}
