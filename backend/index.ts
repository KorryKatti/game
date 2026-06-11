import { Hono } from 'hono'
import { cors } from 'hono/cors'
import { Database } from 'bun:sqlite'

const app = new Hono()
app.use('/*', cors({
  origin: ['http://localhost:5173', 'http://127.0.0.1:5173'],
  credentials: true,
}))
const db = new Database('wizard.db', { create: true })

function genApiKey() {
  return 'wzrd-' + crypto.randomUUID()
}

// 1. Database Init
db.run(`
  CREATE TABLE IF NOT EXISTS users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    username TEXT UNIQUE,
    password TEXT,
    api_key TEXT UNIQUE,
    avatar_url TEXT DEFAULT '',
    elo INTEGER DEFAULT 1200,
    wins INTEGER DEFAULT 0,
    losses INTEGER DEFAULT 0,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
  );
  CREATE TABLE IF NOT EXISTS sessions (
    user_id INTEGER,
    game_ip TEXT,
    game_port INTEGER,
    status TEXT,
    last_seen DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY(user_id) REFERENCES users(id)
  );
  CREATE TABLE IF NOT EXISTS matches (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    match_id TEXT,
    player1 TEXT,
    player2 TEXT,
    winner TEXT,
    duration INTEGER,
    file_path TEXT,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
  );
`)

// 2. Auth (no token needed)
app.post('/auth/register', async (c) => {
  const { username, password, avatar_url } = await c.req.json()
  if (!username || !password) return c.json({ error: 'Username and password required' }, 400)

  const exists = db.query('SELECT id FROM users WHERE username = ?').get(username)
  if (exists) return c.json({ error: 'Username taken' }, 409)

  const apiKey = genApiKey()
  const hashed = Bun.password.hashSync(password)
  db.run('INSERT INTO users (username, password, api_key, avatar_url) VALUES (?, ?, ?, ?)',
    [username, hashed, apiKey, avatar_url || ''])

  return c.json({ success: true, api_key: apiKey })
})

app.post('/auth/login', async (c) => {
  const { username, password } = await c.req.json()
  if (!username || !password) return c.json({ error: 'Username and password required' }, 400)

  const user: any = db.query('SELECT * FROM users WHERE username = ?').get(username)
  if (!user) return c.json({ error: 'User not found' }, 404)

  const valid = Bun.password.verifySync(password, user.password)
  if (!valid) return c.json({ error: 'Wrong password' }, 401)

  return c.json({ success: true, api_key: user.api_key })
})

// 3. Auth Middleware
app.use('/v1/*', async (c, next) => {
  const auth = c.req.header('Authorization')
  if (!auth?.startsWith('Bearer ')) return c.json({ error: 'Missing token' }, 401)
  
  const key = auth.split(' ')[1]
  const user = db.query('SELECT * FROM users WHERE api_key = ?').get(key)
  if (!user) return c.json({ error: 'Invalid key' }, 401)
  
  c.set('user', user)
  await next()
})

// 4. Endpoints
app.get('/v1/profile', (c) => {
  const user: any = c.get('user')
  return c.json({
    username: user.username,
    elo: user.elo,
    wins: user.wins,
    losses: user.losses,
    avatar_url: user.avatar_url,
    created_at: user.created_at
  })
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
  const players = db.query(`
    SELECT users.username, users.elo, s.game_ip, s.game_port
    FROM sessions s
    JOIN users ON s.user_id = users.id
    WHERE s.last_seen > datetime('now', '-5 minutes')
    GROUP BY users.id
  `).all()
  return c.json({ players })
})

// Matches
app.get('/v1/matches', (c) => {
  const user: any = c.get('user')
  const limit = Number(c.req.query('limit')) || 20
  const offset = Number(c.req.query('offset')) || 0
  const matches = db.query(`
    SELECT *,
      CASE WHEN player1 = ? THEN player2 ELSE player1 END as opponent,
      CASE WHEN winner = ? THEN 'win' ELSE 'loss' END as result
    FROM matches
    WHERE player1 = ? OR player2 = ?
    ORDER BY created_at DESC
    LIMIT ? OFFSET ?
  `).all(user.username, user.username, user.username, user.username, limit, offset)
  return c.json({ matches })
})

app.post('/v1/matches', async (c) => {
  const user: any = c.get('user')
  const form = await c.req.formData()
  const matchId = form.get('match_id') as string
  const opponentName = form.get('opponent_name') as string
  const winner = form.get('winner') as string
  const duration = Number(form.get('duration')) || 0
  const file = form.get('match_file') as File

  if (!matchId || !opponentName || !winner) {
    return c.json({ error: 'match_id, opponent_name, winner required' }, 400)
  }

  // Save file
  const dir = `recordings/${matchId}`
  await Bun.write(`${dir}/${user.username}.wzrd`, file)

  // Save match
  db.run(
    'INSERT INTO matches (match_id, player1, player2, winner, duration, file_path) VALUES (?, ?, ?, ?, ?, ?)',
    [matchId, user.username, opponentName, winner, duration, `${dir}/${user.username}.wzrd`]
  )

  // Update wins/losses
  const winCol = winner === user.username ? 'wins' : 'losses'
  db.run(`UPDATE users SET ${winCol} = ${winCol} + 1 WHERE username = ?`, [winner])

  return c.json({ success: true, match_id: matchId })
})

// Leaderboard
app.get('/v1/leaderboard', (c) => {
  const limit = Number(c.req.query('limit')) || 50
  const players = db.query(`
    SELECT username, elo, wins, losses,
      ROW_NUMBER() OVER (ORDER BY elo DESC) as rank
    FROM users
    ORDER BY elo DESC
    LIMIT ?
  `).all(limit)
  return c.json({ players })
})

export default {
  port: 3000,
  fetch: app.fetch,
}
