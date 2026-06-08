import { Database } from 'bun:sqlite'

const db = new Database('wizard.db')
const username = 'Merlin'
const apiKey = 'magic-123'

try {
  db.run('INSERT INTO users (username, api_key) VALUES (?, ?)', [username, apiKey])
  console.log(`User created: ${username} | Key: ${apiKey}`)
} catch (e) {
  console.log('User already exists or DB error.')
}
