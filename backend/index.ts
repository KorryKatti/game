import { Hono } from 'hono'
import { Database } from 'bun:sqlite'

const app = new Hono()
const root = import.meta.dir

app.use('/*', async (c, next) => {
  c.header('Access-Control-Allow-Origin', '*')
  c.header('Access-Control-Allow-Methods', 'GET, POST, PUT, DELETE, OPTIONS')
  c.header('Access-Control-Allow-Headers', '*')
  if (c.req.method === 'OPTIONS') return c.text('', 204)
  await next()
})

const db = new Database(process.env.DB_PATH || `${root}/wizard.db`, { create: true })

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
    user_id INTEGER UNIQUE,
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
  CREATE TABLE IF NOT EXISTS pings (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    from_username TEXT,
    to_username TEXT,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
  );
  CREATE TABLE IF NOT EXISTS chat_messages (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    from_username TEXT,
    to_username TEXT,
    message TEXT,
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

app.get('/v1/health', (c) => {
  console.log('[API] GET /v1/health')
  return c.json({ status: 'ok' })
})

app.get('/health', (c) => {
  console.log('[API] GET /health')
  return c.json({ status: 'ok' })
})

// 3. Auth Middleware
app.use('/v1/*', async (c, next) => {
  const auth = c.req.header('Authorization')
  console.log(`[AUTH] ${c.req.method} ${c.req.path} - Auth header: ${auth ? 'present' : 'missing'}`)
  if (!auth?.startsWith('Bearer ')) return c.json({ error: 'Missing token' }, 401)
  
  const key = auth.split(' ')[1]
  const user = db.query('SELECT * FROM users WHERE api_key = ?').get(key)
  if (!user) {
    console.log(`[AUTH] Invalid key: ${key}`)
    return c.json({ error: 'Invalid key' }, 401)
  }
  
  console.log(`[AUTH] Authenticated as: ${(user as any).username}`)
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
  console.log(`[API] Heartbeat from ${user.username}: ip=${game_ip} port=${game_port} status=${status}`)
  
  db.run(`
    INSERT INTO sessions (user_id, game_ip, game_port, status, last_seen)
    VALUES (?, ?, ?, ?, CURRENT_TIMESTAMP)
    ON CONFLICT(user_id) DO UPDATE SET
      game_ip = excluded.game_ip,
      game_port = excluded.game_port,
      status = excluded.status,
      last_seen = CURRENT_TIMESTAMP
  `, [user.id, game_ip, game_port, status])
  
  return c.json({ success: true })
})

app.get('/v1/players/online', (c) => {
  console.log('[API] GET /v1/players/online')
  const user: any = c.get('user')
  const players = db.query(`
    SELECT users.username, users.elo, users.avatar_url, s.game_ip, s.game_port
    FROM sessions s
    JOIN users ON s.user_id = users.id
    WHERE s.status = 'online' AND s.last_seen > datetime('now', '-5 minutes')
      AND users.username != ?
    GROUP BY users.id
  `).all(user.username)
  console.log(`[API] Online players: ${players.length}`, JSON.stringify(players))
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

  // Update wins/losses for the current player
  const winCol = winner === user.username ? 'wins' : 'losses'
  db.run(`UPDATE users SET ${winCol} = ${winCol} + 1 WHERE username = ?`, [user.username])

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

// Ping
app.post('/v1/players/ping', async (c) => {
  const user: any = c.get('user')
  const { target_username } = await c.req.json()
  if (!target_username) return c.json({ error: 'target_username required' }, 400)

  db.run('INSERT INTO pings (from_username, to_username) VALUES (?, ?)',
    [user.username, target_username])

  return c.json({ success: true })
})

// Chat - send message
app.post('/v1/chat/send', async (c) => {
  const user: any = c.get('user')
  const { target_username, message } = await c.req.json()
  if (!target_username || !message) return c.json({ error: 'target_username and message required' }, 400)

  db.run('INSERT INTO chat_messages (from_username, to_username, message) VALUES (?, ?, ?)',
    [user.username, target_username, message])

  return c.json({ success: true })
})

// Chat - get messages
app.get('/v1/chat/messages', (c) => {
  const user: any = c.get('user')
  const with_user = c.req.query('with')
  const limit = Number(c.req.query('limit')) || 50
  if (!with_user) return c.json({ error: 'with query param required' }, 400)

  const messages = db.query(`
    SELECT * FROM chat_messages
    WHERE (from_username = ? AND to_username = ?)
       OR (from_username = ? AND to_username = ?)
    ORDER BY created_at ASC
    LIMIT ?
  `).all(user.username, with_user, with_user, user.username, limit)

  return c.json({ messages })
})

// 5. Static files (built frontend)
const frontendDir = process.env.FRONTEND_DIR || `${root}/../frontend/dist`
app.get('/*', async (c) => {
  const reqPath = c.req.path === '/' ? '/index.html' : c.req.path
  const file = Bun.file(frontendDir + reqPath)
  if (await file.exists()) return c.body(file)
  return c.notFound()
})

const port = Number(process.env.PORT) || 3000
export default {
  port,
  fetch: app.fetch,
}

console.log(`Server running on http://localhost:${port}`)
