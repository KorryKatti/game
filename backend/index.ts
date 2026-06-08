import { Hono } from 'hono'
import { Database } from 'bun:sqlite'

const app = new Hono()
const db = new Database('wizard.db', { create: true })

// 1. Database Init (Minimal)
db.run(`
  CREATE TABLE IF NOT EXISTS users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    username TEXT UNIQUE,
    api_key TEXT UNIQUE,
    elo INTEGER DEFAULT 1200
  );
  CREATE TABLE IF NOT EXISTS sessions (
    user_id INTEGER,
    game_ip TEXT,
    game_port INTEGER,
    status TEXT,
    last_seen DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY(user_id) REFERENCES users(id)
  );
`)

// 2. Auth Middleware
app.use('/v1/*', async (c, next) => {
  const auth = c.req.header('Authorization')
  if (!auth?.startsWith('Bearer ')) return c.json({ error: 'Missing token' }, 401)
  
  const key = auth.split(' ')[1]
  const user = db.query('SELECT * FROM users WHERE api_key = ?').get(key)
  if (!user) return c.json({ error: 'Invalid key' }, 401)
  
  c.set('user', user)
  await next()
})

// 3. Endpoints
app.get('/v1/profile', (c) => {
  const user: any = c.get('user')
  return c.json({ username: user.username, elo: user.elo })
})

app.post('/v1/players/heartbeat', async (c) => {
  const user: any = c.get('user')
  const { game_ip, game_port, status } = await c.req.json()
  
  db.run(`
    INSERT INTO sessions (user_id, game_ip, game_port, status, last_seen) 
    VALUES (?, ?, ?, ?, CURRENT_TIMESTAMP)
  `, [user.id, game_ip, game_port, status])
  
  return c.json({ success: true })
})

app.get('/v1/players/online', (c) => {
  // Last 5 mins
  const players = db.query(`
    SELECT users.username, users.elo, s.game_ip, s.game_port
    FROM sessions s
    JOIN users ON s.user_id = users.id
    WHERE s.last_seen > datetime('now', '-5 minutes')
    GROUP BY users.id
  `).all()
  
  return c.json({ players })
})

export default {
  port: 3000,
  fetch: app.fetch,
}
