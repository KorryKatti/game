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
                    <a href="/" id="logo">WIZARD DUEL</a>
                    <div class="nav-links">
                        <a href="/dashboard.html" class="${currentPath.includes('dashboard') ? 'active' : ''}">Arena</a>
                        <a href="/leaderboard.html" class="${currentPath.includes('leaderboard') ? 'active' : ''}">Rankings</a>
                        <a href="/docs.html" class="${currentPath.includes('docs') ? 'active' : ''}">Spells</a>
                        ${isLoggedIn
                            ? `<a href="/profile.html" class="${currentPath.includes('profile') ? 'active' : ''}">Sanctum</a>`
                            : `<a href="/login.html">Enter</a>`}
                    </div>
                </div>
            </nav>
        `;
    }

    if (footerPlaceholder) {
        footerPlaceholder.innerHTML = `
            <footer style="border-top: 1px solid var(--color-border); padding: 60px 0; margin-top: 80px;">
                <div class="container">
                    <div class="grid-3" style="font-size: 13px; color: var(--color-text-dim);">
                        <div>
                            <h4 style="font-size: 12px; color: var(--color-gold-dim); margin-bottom: var(--spacing-sm);">THE ARENA</h4>
                            <p style="line-height: 1.8;">Real-time 1v1 wizard combat. Precision, strategy, and mastery define the true duelist.</p>
                        </div>
                        <div>
                            <h4 style="font-size: 12px; color: var(--color-gold-dim); margin-bottom: var(--spacing-sm);">LINKS</h4>
                            <ul style="display: flex; flex-direction: column; gap: 6px;">
                                <li><a href="/docs.html">Spellbook</a></li>
                                <li><a href="/leaderboard.html">Leaderboard</a></li>
                            </ul>
                        </div>
                        <div>
                            <h4 style="font-size: 12px; color: var(--color-gold-dim); margin-bottom: var(--spacing-sm);">STATUS</h4>
                            <p style="line-height: 1.8;">&copy; 2026 &middot; All rights reserved</p>
                        </div>
                    </div>
                </div>
            </footer>
        `;
    }
}
