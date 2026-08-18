/** @type {import('tailwindcss').Config} */
export default {
  content: [
    "./index.html",
    "./src/**/*.{js,ts,jsx,tsx}",
  ],
  darkMode: 'class',
  theme: {
    extend: {
      colors: {
        brand: {
          cyan: '#00C9FF',
          mint: '#92FE9D',
          purple: '#8E2DE2',
          indigo: '#4A00E0',
          dark: '#0A0D14',
          darker: '#06080C',
          card: '#111726',
          cardHover: '#161F33',
          border: '#1F293D',
          borderLight: '#2D3A54',
          accent: '#00E5FF'
        }
      },
      fontFamily: {
        sans: ['Inter', 'system-ui', '-apple-system', 'BlinkMacSystemFont', 'Segoe UI', 'Roboto', 'sans-serif'],
        mono: ['JetBrains Mono', 'Fira Code', 'Consolas', 'monospace'],
      },
      animation: {
        'pulse-slow': 'pulse 3s cubic-bezier(0.4, 0, 0.6, 1) infinite',
        'glow': 'glow 2s ease-in-out infinite alternate',
        'float': 'float 6s ease-in-out infinite',
        'wave': 'wave 1.5s ease-in-out infinite',
      },
      keyframes: {
        glow: {
          '0%': { boxShadow: '0 0 15px rgba(0, 201, 255, 0.3)' },
          '100%': { boxShadow: '0 0 30px rgba(0, 201, 255, 0.8), 0 0 50px rgba(142, 45, 226, 0.4)' }
        },
        float: {
          '0%, 100%': { transform: 'translateY(0px)' },
          '50%': { transform: 'translateY(-10px)' },
        },
        wave: {
          '0%, 100%': { height: '10px' },
          '50%': { height: '36px' },
        }
      }
    },
  },
  plugins: [],
}
