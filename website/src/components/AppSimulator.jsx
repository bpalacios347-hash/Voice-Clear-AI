import React, { useState } from 'react';
import { Mic, MicOff, Settings, Volume2, ShieldCheck, Power, Cpu, Activity, Sliders, Check } from 'lucide-react';

export default function AppSimulator({ t }) {
  const [isActive, setIsActive] = useState(true);
  const [suppressionLevel, setSuppressionLevel] = useState(95);
  const [selectedMic, setSelectedMic] = useState("HyperX QuadCast S (USB Audio)");
  const [autoStart, setAutoStart] = useState(true);

  const microphones = [
    "HyperX QuadCast S (USB Audio)",
    "Blue Yeti Pro (Realtek High Definition)",
    "Elgato Wave:3 (Microphone In)",
    "Rode NT-USB Mini (Core Audio)",
    "Headset Microphone (Realtek Audio)"
  ];

  return (
    <section id="simulator" className="py-24 relative bg-brand-darker">
      <div className="max-w-6xl mx-auto px-4 sm:px-6 lg:px-8">
        
        {/* Section Header */}
        <div className="text-center max-w-3xl mx-auto mb-16">
          <span className="text-xs font-mono font-bold tracking-widest text-brand-mint uppercase px-3 py-1 rounded-full bg-brand-mint/10 border border-brand-mint/20">
            {t.simulator.tag}
          </span>
          <h2 className="text-3xl sm:text-4xl lg:text-5xl font-black text-white mt-4 mb-4 tracking-tight">
            {t.simulator.title}
          </h2>
          <p className="text-slate-300 text-base sm:text-lg">
            {t.simulator.subtitle}
          </p>
        </div>

        {/* Windows App Window Simulation Frame */}
        <div className="max-w-3xl mx-auto rounded-2xl overflow-hidden shadow-2xl border border-slate-700/80 bg-[#0F141F] transition-all duration-300 hover:border-brand-cyan/50">
          
          {/* Windows Titlebar */}
          <div className="bg-[#0A0E17] px-4 py-3 flex items-center justify-between border-b border-slate-800">
            <div className="flex items-center gap-2.5">
              <img src="./logo.svg" alt="Voice Clear AI" className="w-4 h-4" />
              <span className="text-xs font-semibold text-slate-300 font-sans tracking-wide">
                Voice Clear AI — Real-Time Control Center
              </span>
            </div>
            {/* Standard Windows Window Controls */}
            <div className="flex items-center gap-2">
              <span className="w-3 h-0.5 bg-slate-500 rounded cursor-pointer hover:bg-white transition-colors" />
              <span className="w-2.5 h-2.5 border border-slate-500 rounded-[2px] cursor-pointer hover:border-white transition-colors" />
              <span className="text-xs text-slate-500 hover:text-red-400 font-mono cursor-pointer ml-1">✕</span>
            </div>
          </div>

          {/* Main App Content Area */}
          <div className="p-6 sm:p-8 space-y-8">
            
            {/* Big Status & Power Button Area */}
            <div className="flex flex-col items-center justify-center py-6 border-b border-slate-800/80">
              
              {/* Pulsing Toggle Button */}
              <div className="relative mb-4">
                {isActive && (
                  <div className="absolute inset-0 rounded-full bg-brand-cyan/30 animate-ping" />
                )}
                <button
                  onClick={() => setIsActive(!isActive)}
                  className={`relative w-28 h-28 sm:w-32 sm:h-32 rounded-full flex flex-col items-center justify-center transition-all duration-300 shadow-2xl ${
                    isActive
                      ? 'bg-gradient-to-tr from-brand-indigo via-brand-cyan to-brand-mint text-slate-950 shadow-brand-cyan/40 scale-105'
                      : 'bg-slate-800 text-slate-500 border border-slate-700 hover:text-slate-300'
                  }`}
                  title={t.simulator.toggleHint}
                >
                  <Power className={`w-10 h-10 sm:w-12 sm:h-12 ${isActive ? 'stroke-[2.5]' : 'stroke-1'}`} />
                  <span className="text-[11px] font-mono font-bold mt-1 tracking-wider uppercase">
                    {isActive ? 'ACTIVE' : 'STANDBY'}
                  </span>
                </button>
              </div>

              {/* Status Text Indicator */}
              <div className="flex items-center gap-2">
                <span className={`w-2.5 h-2.5 rounded-full ${isActive ? 'bg-brand-mint animate-pulse' : 'bg-slate-600'}`} />
                <span className={`text-sm font-mono font-bold tracking-wide ${isActive ? 'text-brand-mint' : 'text-slate-500'}`}>
                  {isActive ? t.simulator.statusActive : t.simulator.statusInactive}
                </span>
              </div>
              <p className="text-xs text-slate-400 mt-1">{t.simulator.toggleHint}</p>
            </div>

            {/* Input & Output Device Selectors */}
            <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
              
              {/* Physical Input Mic Dropdown */}
              <div className="bg-[#141A28] p-4 rounded-xl border border-slate-800">
                <label className="text-xs font-mono font-semibold text-slate-400 uppercase tracking-wider block mb-2">
                  {t.simulator.micSelect}
                </label>
                <select
                  value={selectedMic}
                  onChange={(e) => setSelectedMic(e.target.value)}
                  className="w-full bg-[#0B0F17] text-white text-xs sm:text-sm font-medium rounded-lg p-2.5 border border-slate-700 focus:border-brand-cyan focus:outline-none"
                >
                  {microphones.map((mic, idx) => (
                    <option key={idx} value={mic}>{mic}</option>
                  ))}
                </select>
              </div>

              {/* Virtual Output Driver */}
              <div className="bg-[#141A28] p-4 rounded-xl border border-slate-800 flex flex-col justify-between">
                <label className="text-xs font-mono font-semibold text-slate-400 uppercase tracking-wider block mb-2">
                  {t.simulator.virtualDevice}
                </label>
                <div className="flex items-center justify-between bg-[#0B0F17] p-2.5 rounded-lg border border-brand-cyan/30 text-brand-cyan">
                  <div className="flex items-center gap-2">
                    <Mic className="w-4 h-4 text-brand-mint" />
                    <span className="text-xs sm:text-sm font-mono font-semibold text-slate-100">
                      {t.simulator.virtualDeviceName}
                    </span>
                  </div>
                  <span className="text-[10px] bg-brand-cyan/20 text-brand-cyan px-2 py-0.5 rounded font-mono">
                    ONLINE
                  </span>
                </div>
              </div>

            </div>

            {/* AI Suppression Intensity & Live VU Meter */}
            <div className="bg-[#141A28] p-4 rounded-xl border border-slate-800 space-y-4">
              <div className="flex items-center justify-between">
                <div className="flex items-center gap-2">
                  <Sliders className="w-4 h-4 text-brand-cyan" />
                  <span className="text-xs font-mono font-semibold text-slate-300">
                    {t.simulator.noiseSuppressionLevel}
                  </span>
                </div>
                <span className="text-xs font-mono font-bold text-brand-mint">
                  {isActive ? `${suppressionLevel}% (-51.5 dB)` : '0% (Bypass)'}
                </span>
              </div>

              <input
                type="range"
                min="0"
                max="100"
                value={isActive ? suppressionLevel : 0}
                disabled={!isActive}
                onChange={(e) => setSuppressionLevel(e.target.value)}
                className="w-full h-2 bg-slate-900 rounded-lg appearance-none cursor-pointer accent-brand-cyan disabled:opacity-40"
              />

              {/* Simulated VU Audio Meter */}
              <div className="space-y-1.5 pt-2">
                <div className="flex items-center justify-between text-[11px] font-mono text-slate-400">
                  <span>Input Signal VU:</span>
                  <span className={isActive ? 'text-brand-mint' : 'text-slate-500'}>
                    {isActive ? '-14.2 dBFS (Clean Voice)' : '-4.1 dBFS (Noisy Peak)'}
                  </span>
                </div>
                <div className="w-full h-3 bg-slate-900 rounded-full overflow-hidden p-0.5 flex gap-1">
                  {[...Array(24)].map((_, i) => (
                    <div
                      key={i}
                      className={`flex-1 rounded-[1px] transition-all duration-150 ${
                        isActive
                          ? (i < (suppressionLevel / 5) ? (i > 18 ? 'bg-red-400' : (i > 12 ? 'bg-yellow-400' : 'bg-brand-cyan')) : 'bg-slate-800')
                          : (i < 20 ? 'bg-red-500 animate-pulse' : 'bg-slate-800')
                      }`}
                    />
                  ))}
                </div>
              </div>
            </div>

            {/* Performance Footer Bar */}
            <div className="grid grid-cols-3 gap-2 sm:gap-4 pt-2 text-center text-xs font-mono">
              <div className="bg-[#0B0F17] p-2.5 rounded-lg border border-slate-800">
                <span className="text-slate-500 block text-[10px]">{t.simulator.cpuUsage}</span>
                <span className="text-brand-cyan font-bold font-mono">0.8% (AVX2)</span>
              </div>
              <div className="bg-[#0B0F17] p-2.5 rounded-lg border border-slate-800">
                <span className="text-slate-500 block text-[10px]">{t.simulator.latencyText}</span>
                <span className="text-brand-mint font-bold font-mono">9.83 ms</span>
              </div>
              <div className="bg-[#0B0F17] p-2.5 rounded-lg border border-slate-800">
                <span className="text-slate-500 block text-[10px]">Sample Buffer</span>
                <span className="text-yellow-400 font-bold font-mono">480 @ 48kHz</span>
              </div>
            </div>

            {/* Background Service Toggle */}
            <div className="flex items-center justify-between text-xs text-slate-400 pt-2 border-t border-slate-800">
              <div className="flex items-center gap-2">
                <ShieldCheck className="w-4 h-4 text-brand-cyan" />
                <span>{t.simulator.runInBackground}</span>
              </div>
              <button
                onClick={() => setAutoStart(!autoStart)}
                className={`w-10 h-5 rounded-full p-0.5 transition-colors ${
                  autoStart ? 'bg-brand-cyan' : 'bg-slate-700'
                }`}
              >
                <div className={`w-4 h-4 rounded-full bg-slate-950 transition-transform ${
                  autoStart ? 'translate-x-5' : 'translate-x-0'
                }`} />
              </button>
            </div>

          </div>
        </div>

      </div>
    </section>
  );
}
