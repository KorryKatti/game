import WizardAPI from './api.js';

export function injectUI() {
    const navPlaceholder = document.getElementById('nav-placeholder');
    const footerPlaceholder = document.getElementById('footer-placeholder');

    const currentPath = window.location.pathname;
    const isLoggedIn = !!WizardAPI.getApiKey();

    if (navPlaceholder) {
        navPlaceholder.innerHTML = `
            <nav id="main-nav">
                <div class="container nav-content">
                    <a href="/" class="font-mono" id="logo">WIZARD DUEL</a>
                    <div class="nav-links">
                        <a href="/dashboard.html" class="${currentPath.includes('dashboard') ? 'active' : ''}">Dashboard</a>
                        <a href="/leaderboard.html" class="${currentPath.includes('leaderboard') ? 'active' : ''}">Leaderboard</a>
                        <a href="/docs.html" class="${currentPath.includes('docs') ? 'active' : ''}">Docs</a>
                        ${isLoggedIn ? `<a href="/profile.html" class="${currentPath.includes('profile') ? 'active' : ''}">Profile</a>` : `<a href="/login.html">Login</a>`}
                        <button id="theme-toggle" class="btn" style="padding: 2px 8px; font-size: 10px; margin-left: var(--spacing-sm);">DARK MODE</button>
                    </div>
                </div>
            </nav>
        `;

        const toggle = document.getElementById('theme-toggle');
        if (toggle) {
            toggle.addEventListener('click', () => {
                const isDark = document.body.classList.toggle('dark-mode');
                localStorage.setItem('wzrd_theme', isDark ? 'dark' : 'light');
                toggle.textContent = isDark ? 'LIGHT MODE' : 'DARK MODE';
            });
            
            if (localStorage.getItem('wzrd_theme') === 'dark') {
                document.body.classList.add('dark-mode');
                toggle.textContent = 'LIGHT MODE';
            }
        }
    }

    if (footerPlaceholder) {
        footerPlaceholder.innerHTML = `
            <footer style="border-top: 1px solid var(--color-border); padding: 80px 0; margin-top: 100px; background: var(--color-light-gray);">
                <div class="container">
                    <div style="background: var(--color-red); color: white; padding: 15px; font-size: 12px; margin-bottom: 60px; text-align: center; font-family: var(--font-display); font-weight: 700; letter-spacing: 0.1em; border: 2px solid var(--color-text);">
                        NOTICE: THIS INTERFACE IS TEMPORARY AND SUBJECT TO CHANGE UPON FINAL RELEASE.
                    </div>
                    <div class="grid-3" style="font-size: 12px; opacity: 0.8;">
                        <div>
                            <h4 style="font-size: 14px; margin-bottom: var(--spacing-sm);">SYSTEM</h4>
                            <p>A technical 1v1 dueling platform designed for competitive clarity and precision.</p>
                        </div>
                        <div>
                            <h4 style="font-size: 14px; margin-bottom: var(--spacing-sm);">RESOURCES</h4>
                            <ul style="display: flex; flex-direction: column; gap: 8px;">
                                <li><a href="/docs.html">Documentation</a></li>
                                <li><a href="/leaderboard.html">Leaderboard</a></li>
                            </ul>
                        </div>
                        <div>
                            <h4 style="font-size: 14px; margin-bottom: var(--spacing-sm);">LEGAL</h4>
                            <p>&copy; 2026 WIZARD DUEL. ALL DATA PROTECTED.</p>
                        </div>
                    </div>
                </div>
            </footer>
        `;
    }
}
