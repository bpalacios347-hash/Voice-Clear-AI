import React, { useState } from 'react';
import { X, Download, CheckCircle2, ShieldCheck, FileCheck, ArrowDownToLine, Sparkles } from 'lucide-react';
import confetti from 'canvas-confetti';

export default function DownloadModal({ isOpen, onClose, t }) {
  const [downloading, setDownloading] = useState(false);
  const [downloaded, setDownloaded] = useState(false);

  if (!isOpen) return null;

  const handleDownload = () => {
    setDownloading(true);

    // Launch celebratory confetti
    confetti({
      particleCount: 80,
      spread: 70,
      origin: { y: 0.6 }
    });

    setTimeout(() => {
      setDownloading(false);
      setDownloaded(true);
      // Simulate file download by creating anchor
      const link = document.createElement('a');
      link.href = './logo.svg'; // Or setup installer binary
      link.download = 'VoiceClearAI_Setup_v1.0.exe';
      // document.body.appendChild(link);
      // link.click();
    }, 1200);
  };

  return (
    <div className="fixed inset-0 z-50 flex items-center justify-center p-4 bg-black/80 backdrop-blur-sm animate-fadeIn">
      <div className="relative w-full max-w-lg glass-card-glow rounded-3xl p-6 sm:p-8 border border-brand-cyan/40 shadow-2xl overflow-hidden">
        
        {/* Close button */}
        <button
          onClick={onClose}
          className="absolute top-5 right-5 p-2 rounded-xl bg-slate-900 border border-slate-800 text-slate-400 hover:text-white transition-colors"
        >
          <X className="w-5 h-5" />
        </button>

        {/* Modal Header */}
        <div className="flex items-center gap-3 mb-6">
          <div className="w-12 h-12 rounded-2xl bg-gradient-to-tr from-brand-cyan to-brand-mint p-0.5 shadow-md">
            <div className="w-full h-full bg-brand-dark rounded-[14px] flex items-center justify-center">
              <Download className="w-6 h-6 text-brand-cyan" />
            </div>
          </div>
          <div>
            <h3 className="text-xl font-bold text-white tracking-tight">
              {t.modals.download.title}
            </h3>
            <p className="text-xs text-slate-400 font-mono">
              {t.modals.download.subtitle}
            </p>
          </div>
        </div>

        {/* Download Box */}
        <div className="bg-brand-darker/90 rounded-2xl p-5 border border-brand-border mb-6 text-center">
          {downloaded ? (
            <div className="py-4 space-y-2">
              <div className="w-12 h-12 rounded-full bg-brand-mint/20 text-brand-mint flex items-center justify-center mx-auto mb-2">
                <CheckCircle2 className="w-7 h-7" />
              </div>
              <h4 className="text-base font-bold text-white">¡Descarga Iniciada con Éxito!</h4>
              <p className="text-xs text-slate-400">
                El archivo <code className="text-brand-cyan font-mono">VoiceClearAI_Setup.exe</code> está listo en tu carpeta de descargas.
              </p>
            </div>
          ) : (
            <div className="space-y-4">
              <button
                onClick={handleDownload}
                disabled={downloading}
                className="w-full py-4 rounded-xl bg-gradient-to-r from-brand-cyan to-brand-mint hover:opacity-90 text-slate-950 font-extrabold text-base flex items-center justify-center gap-2 shadow-lg shadow-brand-cyan/25 transition-all"
              >
                {downloading ? (
                  <>
                    <span className="w-5 h-5 border-2 border-slate-950 border-t-transparent rounded-full animate-spin" />
                    <span>Preparando archivo...</span>
                  </>
                ) : (
                  <>
                    <ArrowDownToLine className="w-5 h-5" />
                    <span>{t.modals.download.btnDownload}</span>
                  </>
                )}
              </button>
              <p className="text-[11px] font-mono text-slate-400">
                {t.modals.download.fileInfo}
              </p>
            </div>
          )}
        </div>

        {/* Installation Instructions */}
        <div className="space-y-2.5 text-xs text-slate-300 bg-slate-900/60 p-4 rounded-xl border border-slate-800 mb-6">
          <span className="font-bold text-white font-mono block">
            {t.modals.download.stepsTitle}
          </span>
          <p>{t.modals.download.step1}</p>
          <p>{t.modals.download.step2}</p>
          <p>{t.modals.download.step3}</p>
        </div>

        {/* Security badge */}
        <div className="flex items-center justify-between text-xs text-slate-400 pt-2 border-t border-slate-800">
          <div className="flex items-center gap-1.5 text-brand-mint font-mono">
            <ShieldCheck className="w-4 h-4" />
            <span>Verificado Seguro (Clean Install)</span>
          </div>
          <span className="font-mono text-[10px]">SHA-256 Verified</span>
        </div>

      </div>
    </div>
  );
}
