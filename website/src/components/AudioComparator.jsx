import React, { useState, useRef, useEffect } from 'react';
import { Play, Pause, Volume2, VolumeX, Sparkles, AlertCircle, CheckCircle2, RotateCcw, Activity } from 'lucide-react';

export default function AudioComparator({ t }) {
  const [isPlaying, setIsPlaying] = useState(false);
  const [isClean, setIsClean] = useState(true); // true = Clean, false = Noisy
  const [currentTime, setCurrentTime] = useState(0);
  const [duration, setDuration] = useState(10);
  const [volume, setVolume] = useState(0.85);

  const noisyAudioRef = useRef(null);
  const cleanAudioRef = useRef(null);
  const canvasRef = useRef(null);
  const animFrameId = useRef(null);

  // Sync state between both audio elements
  useEffect(() => {
    const noisy = noisyAudioRef.current;
    const clean = cleanAudioRef.current;

    if (!noisy || !clean) return;

    noisy.volume = isClean ? 0 : volume;
    clean.volume = isClean ? volume : 0;

    const handleLoadedMetadata = () => {
      if (clean.duration && !isNaN(clean.duration)) {
        setDuration(clean.duration);
      }
    };

    const handleTimeUpdate = () => {
      const activeAudio = isClean ? clean : noisy;
      setCurrentTime(activeAudio.currentTime);
    };

    const handleEnded = () => {
      setIsPlaying(false);
      clean.currentTime = 0;
      noisy.currentTime = 0;
      setCurrentTime(0);
    };

    clean.addEventListener('loadedmetadata', handleLoadedMetadata);
    clean.addEventListener('timeupdate', handleTimeUpdate);
    clean.addEventListener('ended', handleEnded);

    return () => {
      clean.removeEventListener('loadedmetadata', handleLoadedMetadata);
      clean.removeEventListener('timeupdate', handleTimeUpdate);
      clean.removeEventListener('ended', handleEnded);
    };
  }, [isClean, volume]);

  // Handle Play/Pause
  const togglePlay = async () => {
    const noisy = noisyAudioRef.current;
    const clean = cleanAudioRef.current;

    if (!noisy || !clean) return;

    if (isPlaying) {
      noisy.pause();
      clean.pause();
      setIsPlaying(false);
    } else {
      // Ensure sync before play
      const targetTime = currentTime;
      noisy.currentTime = targetTime;
      clean.currentTime = targetTime;
      
      noisy.volume = isClean ? 0 : volume;
      clean.volume = isClean ? volume : 0;

      try {
        await Promise.all([noisy.play(), clean.play()]);
        setIsPlaying(true);
      } catch (err) {
        console.warn("Audio playback interrupted or failed:", err);
      }
    }
  };

  // Switch between clean and noisy smoothly
  const handleToggleMode = (mode) => {
    const cleanTarget = mode === 'clean';
    setIsClean(cleanTarget);

    const noisy = noisyAudioRef.current;
    const clean = cleanAudioRef.current;

    if (noisy && clean) {
      if (cleanTarget) {
        clean.currentTime = noisy.currentTime;
        clean.volume = volume;
        noisy.volume = 0;
      } else {
        noisy.currentTime = clean.currentTime;
        noisy.volume = volume;
        clean.volume = 0;
      }
    }
  };

  // Seek bar
  const handleSeek = (e) => {
    const seekTime = parseFloat(e.target.value);
    setCurrentTime(seekTime);
    if (noisyAudioRef.current && cleanAudioRef.current) {
      noisyAudioRef.current.currentTime = seekTime;
      cleanAudioRef.current.currentTime = seekTime;
    }
  };

  // Canvas Waveform Animation
  useEffect(() => {
    const canvas = canvasRef.current;
    if (!canvas) return;
    const ctx = canvas.getContext('2d');

    const renderWave = () => {
      ctx.clearRect(0, 0, canvas.width, canvas.height);
      const width = canvas.width;
      const height = canvas.height;
      const centerY = height / 2;
      const bars = 48;
      const barWidth = width / bars;

      for (let i = 0; i < bars; i++) {
        let barHeight;
        if (isPlaying) {
          const freq = (i * 0.2) + (Date.now() * 0.005);
          if (isClean) {
            // Smooth vocal harmonic waves
            barHeight = Math.sin(freq) * 20 + Math.cos(freq * 1.5) * 15 + 28;
          } else {
            // Chaotic noisy spikes
            barHeight = Math.random() * 45 + Math.sin(freq * 3) * 20 + 25;
          }
        } else {
          barHeight = 6;
        }

        const x = i * barWidth;
        const gradient = ctx.createLinearGradient(0, centerY - barHeight, 0, centerY + barHeight);
        
        if (isClean) {
          gradient.addColorStop(0, '#00C9FF');
          gradient.addColorStop(1, '#92FE9D');
        } else {
          gradient.addColorStop(0, '#EF4444');
          gradient.addColorStop(1, '#F97316');
        }

        ctx.fillStyle = gradient;
        ctx.beginPath();
        ctx.roundRect(x + 2, centerY - (barHeight / 2), barWidth - 4, barHeight, 3);
        ctx.fill();
      }

      animFrameId.current = requestAnimationFrame(renderWave);
    };

    renderWave();

    return () => {
      if (animFrameId.current) cancelAnimationFrame(animFrameId.current);
    };
  }, [isPlaying, isClean]);

  const formatTime = (secs) => {
    const m = Math.floor(secs / 60);
    const s = Math.floor(secs % 60);
    return `${m}:${s < 10 ? '0' : ''}${s}`;
  };

  return (
    <section id="demo" className="py-24 relative bg-brand-dark overflow-hidden">
      {/* Background radial glow */}
      <div className="absolute inset-0 bg-radial-glow opacity-60 pointer-events-none" />

      {/* Hidden audio tags */}
      <audio ref={noisyAudioRef} src="./audio/noisy_speech.wav" preload="auto" />
      <audio ref={cleanAudioRef} src="./audio/enhanced_speech.wav" preload="auto" />

      <div className="max-w-5xl mx-auto px-4 sm:px-6 lg:px-8 relative">
        
        {/* Section Header */}
        <div className="text-center max-w-3xl mx-auto mb-12">
          <span className="text-xs font-mono font-bold tracking-widest text-brand-cyan uppercase px-3 py-1 rounded-full bg-brand-cyan/10 border border-brand-cyan/20">
            {t.comparator.tag}
          </span>
          <h2 className="text-3xl sm:text-4xl lg:text-5xl font-black text-white mt-4 mb-4 tracking-tight">
            {t.comparator.title}
          </h2>
          <p className="text-slate-300 text-base sm:text-lg">
            {t.comparator.subtitle}
          </p>
        </div>

        {/* Interactive Player Card */}
        <div className="glass-card-glow rounded-3xl p-6 sm:p-10 shadow-2xl border border-brand-border relative overflow-hidden">
          
          {/* Top Mode Selector Tabs */}
          <div className="grid grid-cols-2 gap-3 p-1.5 bg-brand-darker/90 rounded-2xl border border-brand-border mb-8 max-w-lg mx-auto">
            <button
              onClick={() => handleToggleMode('noisy')}
              className={`flex items-center justify-center gap-2 py-3 px-4 rounded-xl text-xs sm:text-sm font-bold transition-all ${
                !isClean 
                  ? 'bg-red-500/20 text-red-400 border border-red-500/40 shadow-lg shadow-red-500/10' 
                  : 'text-slate-400 hover:text-slate-200'
              }`}
            >
              <AlertCircle className="w-4 h-4" />
              <span>{t.comparator.modeNoisy}</span>
            </button>

            <button
              onClick={() => handleToggleMode('clean')}
              className={`flex items-center justify-center gap-2 py-3 px-4 rounded-xl text-xs sm:text-sm font-bold transition-all ${
                isClean 
                  ? 'bg-brand-cyan/20 text-brand-cyan border border-brand-cyan/40 shadow-lg shadow-brand-cyan/20' 
                  : 'text-slate-400 hover:text-slate-200'
              }`}
            >
              <Sparkles className="w-4 h-4 text-brand-mint" />
              <span>{t.comparator.modeClean}</span>
            </button>
          </div>

          {/* Real-Time Waveform Visualizer Canvas */}
          <div className="relative w-full h-32 sm:h-40 bg-brand-darker/95 rounded-2xl border border-brand-border p-4 mb-8 flex flex-col justify-between overflow-hidden">
            <div className="flex items-center justify-between z-10">
              <div className="flex items-center gap-2">
                <span className={`h-2.5 w-2.5 rounded-full ${isPlaying ? (isClean ? 'bg-brand-mint animate-pulse' : 'bg-red-500 animate-pulse') : 'bg-slate-600'}`} />
                <span className="text-xs font-mono font-semibold uppercase tracking-wider text-slate-300">
                  {isPlaying ? (isClean ? 'DeepFilterNet3 Active' : 'Raw Audio Input') : 'Ready to Play'}
                </span>
              </div>
              <div className="flex items-center gap-2 text-xs font-mono text-slate-400">
                <Activity className="w-3.5 h-3.5 text-brand-cyan" />
                <span>48,000 Hz / 24-bit</span>
              </div>
            </div>

            <canvas
              ref={canvasRef}
              width={600}
              height={100}
              className="w-full h-full absolute inset-0 pointer-events-none opacity-85"
            />

            <div className="flex items-center justify-between z-10 text-[11px] font-mono text-slate-400">
              <span>{formatTime(currentTime)}</span>
              <span>{formatTime(duration)}</span>
            </div>
          </div>

          {/* Audio Scrubber Timeline */}
          <div className="mb-8">
            <input
              type="range"
              min="0"
              max={duration || 10}
              step="0.05"
              value={currentTime}
              onChange={handleSeek}
              className="w-full h-2 bg-slate-800 rounded-lg appearance-none cursor-pointer accent-brand-cyan"
            />
            <div className="flex justify-between text-xs text-slate-500 font-mono mt-1">
              <span>0:00</span>
              <span className="text-slate-400 italic">{t.comparator.sliderHint}</span>
              <span>{formatTime(duration)}</span>
            </div>
          </div>

          {/* Controls Bar */}
          <div className="flex flex-col sm:flex-row items-center justify-between gap-6 pt-4 border-t border-brand-border">
            
            {/* Play/Pause & Reset Buttons */}
            <div className="flex items-center gap-4">
              <button
                onClick={togglePlay}
                className="w-14 h-14 rounded-2xl bg-gradient-to-tr from-brand-cyan to-brand-mint text-slate-950 flex items-center justify-center shadow-xl shadow-brand-cyan/30 hover:scale-105 active:scale-95 transition-all"
                title={isPlaying ? t.comparator.paused : t.comparator.playing}
              >
                {isPlaying ? (
                  <Pause className="w-6 h-6 fill-current" />
                ) : (
                  <Play className="w-6 h-6 fill-current ml-0.5" />
                )}
              </button>

              <div className="flex flex-col">
                <span className="text-sm font-bold text-white flex items-center gap-2">
                  {isPlaying ? t.comparator.playing : t.comparator.paused}
                  {isPlaying && isClean && (
                    <span className="text-[10px] px-2 py-0.5 rounded-full bg-brand-cyan/20 text-brand-cyan font-mono border border-brand-cyan/40">
                      -51.58 dB
                    </span>
                  )}
                </span>
                <span className="text-xs text-slate-400 font-mono">
                  {isClean ? t.comparator.modeClean : t.comparator.modeNoisy}
                </span>
              </div>
            </div>

            {/* Live Toggle Pill Button */}
            <div className="flex items-center gap-3 bg-brand-darker px-4 py-2 rounded-2xl border border-brand-border">
              <span className="text-xs text-slate-400 font-medium">{t.comparator.clickToToggle}:</span>
              <button
                onClick={() => handleToggleMode(isClean ? 'noisy' : 'clean')}
                className={`px-4 py-1.5 rounded-xl font-mono text-xs font-bold transition-all ${
                  isClean 
                    ? 'bg-gradient-to-r from-brand-cyan to-brand-mint text-slate-950 shadow-md shadow-brand-cyan/20' 
                    : 'bg-red-500/20 text-red-400 border border-red-500/50'
                }`}
              >
                {isClean ? "✓ IA Activada (Limpio)" : "✕ IA Desactivada (Ruido)"}
              </button>
            </div>

          </div>

          {/* Benchmark Specs Pill Bar */}
          <div className="mt-8 pt-6 border-t border-brand-border/60 grid grid-cols-1 sm:grid-cols-3 gap-3 text-center">
            <div className="flex items-center justify-center gap-2 text-xs font-mono text-slate-300">
              <CheckCircle2 className="w-4 h-4 text-brand-mint" />
              <span>{t.comparator.specsBanner.attenuation}</span>
            </div>
            <div className="flex items-center justify-center gap-2 text-xs font-mono text-slate-300">
              <CheckCircle2 className="w-4 h-4 text-brand-cyan" />
              <span>{t.comparator.specsBanner.engine}</span>
            </div>
            <div className="flex items-center justify-center gap-2 text-xs font-mono text-slate-300">
              <CheckCircle2 className="w-4 h-4 text-yellow-400" />
              <span>{t.comparator.specsBanner.latency}</span>
            </div>
          </div>

        </div>

      </div>
    </section>
  );
}
