import { resolve, dirname } from 'path'
import { defineConfig } from 'vite'
import { fileURLToPath } from 'url'

const __filename = fileURLToPath(import.meta.url)
const __dirname = dirname(__filename)

export default defineConfig({
    build: {
        rollupOptions: {
            input: {
                main: resolve(__dirname, 'index.html'),
                dashboard: resolve(__dirname, 'dashboard.html'),
                leaderboard: resolve(__dirname, 'leaderboard.html'),
                login: resolve(__dirname, 'login.html'),
                register: resolve(__dirname, 'register.html'),
                profile: resolve(__dirname, 'profile.html'),
                docs: resolve(__dirname, 'docs.html'),
            },
        },
    },
    server: {
        proxy: {
            '/auth': 'http://localhost:3000',
            '/v1': 'http://localhost:3000',
        },
    },
})
