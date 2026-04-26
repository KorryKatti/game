const BASE_URL = import.meta.env.VITE_API_URL || 'http://localhost:3000';

class WizardAPI {
    static getApiKey() {
        return localStorage.getItem('wzrd_api_key');
    }

    static setApiKey(key) {
        localStorage.setItem('wzrd_api_key', key);
    }

    static clearApiKey() {
        localStorage.removeItem('wzrd_api_key');
    }

    static async request(endpoint, options = {}) {
        const url = `${BASE_URL}${endpoint}`;
        const headers = {
            'Content-Type': 'application/json',
            ...options.headers,
        };

        const apiKey = this.getApiKey();
        if (apiKey) {
            headers['X-API-Key'] = apiKey;
        }

        const response = await fetch(url, { ...options, headers });
        const data = await response.json();

        if (!response.ok) {
            throw new Error(data.error || 'API Request Failed');
        }

        return data;
    }

    // Auth
    static login(username, password) {
        return this.request('/auth/login', {
            method: 'POST',
            body: JSON.stringify({ username, password })
        });
    }

    static register(username, password, avatarUrl = '') {
        return this.request('/auth/register', {
            method: 'POST',
            body: JSON.stringify({ username, password, avatar_url: avatarUrl })
        });
    }

    // Profile
    static getProfile() {
        return this.request('/profile');
    }

    // Matches
    static getMatches(limit = 20, offset = 0) {
        return this.request(`/matches?limit=${limit}&offset=${offset}`);
    }

    static uploadMatch(formData) {
        // Note: For multipart/form-data, don't set Content-Type manually, 
        // fetch will set it with the boundary.
        const apiKey = this.getApiKey();
        const headers = apiKey ? { 'X-API-Key': apiKey } : {};
        
        return fetch(`${BASE_URL}/matches`, {
            method: 'POST',
            headers,
            body: formData
        }).then(res => res.json());
    }

    // Players
    static getOnlinePlayers() {
        return this.request('/players/online');
    }

    static heartbeat(status = 'online', ip = '127.0.0.1', port = 7777) {
        return this.request('/players/heartbeat', {
            method: 'POST',
            body: JSON.stringify({ game_ip: ip, game_port: port, status })
        });
    }

    // Leaderboard
    static getLeaderboard(limit = 50) {
        return this.request(`/leaderboard?limit=${limit}`);
    }
}

export default WizardAPI;
