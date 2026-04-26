import './style.css';

// Minimalist Duelists - Core Logic
console.log('Minimalist Duelists initialized.');

// In a real app, this would handle page transitions, state, etc.
// For now, it just adds some subtle hover effects or logs.

document.addEventListener('DOMContentLoaded', () => {
  const heroBtn = document.querySelector('.hero .btn');
  if (heroBtn) {
    heroBtn.addEventListener('click', (e) => {
      // Subtle click animation handled by CSS active state
      console.log('Entering Arena...');
    });
  }
});
