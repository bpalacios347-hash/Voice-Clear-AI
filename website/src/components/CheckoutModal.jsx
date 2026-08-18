import React, { useState } from 'react';
import { X, ShieldCheck, CheckCircle2, Lock, CreditCard, Sparkles, Copy, Check } from 'lucide-react';
import confetti from 'canvas-confetti';

export default function CheckoutModal({ isOpen, onClose, selectedPlanId = 'pro', t }) {
  const [email, setEmail] = useState('');
  const [paymentMethod, setPaymentMethod] = useState('card');
  const [processing, setProcessing] = useState(false);
  const [success, setSuccess] = useState(false);
  const [generatedKey, setGeneratedKey] = useState('');
  const [copied, setCopied] = useState(false);

  if (!isOpen) return null;

  const planInfo = selectedPlanId === 'studio' 
    ? { name: 'Studio & Commercial License', price: '$69 USD', period: 'Vitalicia (5 PCs)' }
    : { name: 'Pro Lifetime License', price: '$29 USD', period: 'Vitalicia (2 PCs)' };

  const handleCheckout = (e) => {
    e.preventDefault();
    if (!email) return;

    setProcessing(true);

    // Simulate API license generation
    setTimeout(() => {
      setProcessing(false);
      setSuccess(true);
      const randomKey = `VCAI-PRO-${Math.random().toString(36).substring(2, 6).toUpperCase()}-${Math.random().toString(36).substring(2, 6).toUpperCase()}-${Math.random().toString(36).substring(2, 6).toUpperCase()}`;
      setGeneratedKey(randomKey);

      confetti({
        particleCount: 120,
        spread: 80,
        origin: { y: 0.5 }
      });
    }, 1500);
  };

  const handleCopyKey = () => {
    navigator.clipboard.writeText(generatedKey);
    setCopied(true);
    setTimeout(() => setCopied(false), 2000);
  };

  return (
    <div className="fixed inset-0 z-50 flex items-center justify-center p-4 bg-black/85 backdrop-blur-md animate-fadeIn">
      <div className="relative w-full max-w-lg glass-card-glow rounded-3xl p-6 sm:p-8 border border-brand-cyan/50 shadow-2xl overflow-hidden">
        
        {/* Close button */}
        <button
          onClick={onClose}
          className="absolute top-5 right-5 p-2 rounded-xl bg-slate-900 border border-slate-800 text-slate-400 hover:text-white transition-colors"
        >
          <X className="w-5 h-5" />
        </button>

        {/* Modal Header */}
        <div className="mb-6">
          <div className="flex items-center gap-2 text-brand-mint text-xs font-mono font-bold uppercase mb-1">
            <Lock className="w-3.5 h-3.5" />
            <span>Pago Seguro Encriptado SSL</span>
          </div>
          <h3 className="text-2xl font-black text-white tracking-tight">
            {t.modals.checkout.title}
          </h3>
          <p className="text-xs text-slate-400">
            {t.modals.checkout.subtitle}
          </p>
        </div>

        {success ? (
          /* Post-Purchase Success State */
          <div className="space-y-6 text-center py-4">
            <div className="w-16 h-16 rounded-full bg-brand-mint/20 text-brand-mint flex items-center justify-center mx-auto">
              <CheckCircle2 className="w-10 h-10" />
            </div>

            <div>
              <h4 className="text-xl font-bold text-white mb-1">¡Gracias por tu compra!</h4>
              <p className="text-xs text-slate-300">
                Hemos enviado la factura y el instalador a <strong className="text-brand-cyan">{email}</strong>.
              </p>
            </div>

            {/* License Key Box */}
            <div className="bg-brand-darker p-4 rounded-2xl border border-brand-cyan/40 text-left">
              <span className="text-[11px] font-mono text-slate-400 block mb-1">
                Tu Clave de Activación Vitalicia:
              </span>
              <div className="flex items-center justify-between gap-2">
                <code className="text-base font-mono font-bold text-brand-cyan tracking-wider">
                  {generatedKey}
                </code>
                <button
                  onClick={handleCopyKey}
                  className="px-3 py-1.5 rounded-lg bg-slate-800 hover:bg-slate-700 text-xs font-mono text-white flex items-center gap-1.5 transition-colors"
                >
                  {copied ? <Check className="w-3.5 h-3.5 text-brand-mint" /> : <Copy className="w-3.5 h-3.5" />}
                  <span>{copied ? 'Copiado' : 'Copiar'}</span>
                </button>
              </div>
            </div>

            <button
              onClick={onClose}
              className="w-full py-3.5 rounded-xl bg-gradient-to-r from-brand-cyan to-brand-mint text-slate-950 font-bold text-sm"
            >
              Cerrar y Comenzar a Usar
            </button>
          </div>
        ) : (
          /* Purchase Form */
          <form onSubmit={handleCheckout} className="space-y-5">
            
            {/* Plan Summary Card */}
            <div className="bg-brand-darker/90 p-4 rounded-2xl border border-brand-border flex items-center justify-between">
              <div>
                <span className="text-[10px] font-mono text-slate-400 uppercase tracking-wider block">
                  {t.modals.checkout.planLabel}
                </span>
                <span className="font-bold text-sm sm:text-base text-white">
                  {planInfo.name}
                </span>
                <span className="text-xs text-brand-mint font-mono block">
                  {planInfo.period}
                </span>
              </div>
              <div className="text-right">
                <span className="text-2xl font-black font-mono text-brand-cyan">
                  {planInfo.price}
                </span>
              </div>
            </div>

            {/* Email Input */}
            <div>
              <label className="block text-xs font-mono font-semibold text-slate-300 mb-1.5 uppercase">
                Correo Electrónico (Para recibir tu clave de licencia)
              </label>
              <input
                type="email"
                required
                value={email}
                onChange={(e) => setEmail(e.target.value)}
                placeholder={t.modals.checkout.emailPlaceholder}
                className="w-full bg-brand-darker border border-slate-700 rounded-xl p-3 text-sm text-white placeholder-slate-500 focus:border-brand-cyan focus:outline-none"
              />
            </div>

            {/* Payment Method Selector */}
            <div className="grid grid-cols-2 gap-3">
              <button
                type="button"
                onClick={() => setPaymentMethod('card')}
                className={`py-2.5 px-3 rounded-xl text-xs font-bold font-mono flex items-center justify-center gap-2 border transition-all ${
                  paymentMethod === 'card'
                    ? 'bg-brand-cyan/15 border-brand-cyan text-brand-cyan'
                    : 'bg-brand-darker border-slate-800 text-slate-400'
                }`}
              >
                <CreditCard className="w-4 h-4" />
                <span>Tarjeta Crédito/Débito</span>
              </button>

              <button
                type="button"
                onClick={() => setPaymentMethod('paypal')}
                className={`py-2.5 px-3 rounded-xl text-xs font-bold font-mono flex items-center justify-center gap-2 border transition-all ${
                  paymentMethod === 'paypal'
                    ? 'bg-brand-cyan/15 border-brand-cyan text-brand-cyan'
                    : 'bg-brand-darker border-slate-800 text-slate-400'
                }`}
              >
                <span>PayPal Express</span>
              </button>
            </div>

            {/* Card details placeholder if card selected */}
            {paymentMethod === 'card' && (
              <div className="space-y-3 pt-1">
                <input
                  type="text"
                  placeholder={t.modals.checkout.cardPlaceholder}
                  defaultValue="4242 •••• •••• 4242"
                  className="w-full bg-brand-darker border border-slate-700 rounded-xl p-3 text-xs sm:text-sm text-white focus:border-brand-cyan focus:outline-none font-mono"
                />
                <div className="grid grid-cols-2 gap-3">
                  <input
                    type="text"
                    placeholder={t.modals.checkout.expiryPlaceholder}
                    defaultValue="12/28"
                    className="bg-brand-darker border border-slate-700 rounded-xl p-3 text-xs sm:text-sm text-white focus:border-brand-cyan focus:outline-none font-mono text-center"
                  />
                  <input
                    type="text"
                    placeholder={t.modals.checkout.cvcPlaceholder}
                    defaultValue="888"
                    className="bg-brand-darker border border-slate-700 rounded-xl p-3 text-xs sm:text-sm text-white focus:border-brand-cyan focus:outline-none font-mono text-center"
                  />
                </div>
              </div>
            )}

            {/* Submit Button */}
            <button
              type="submit"
              disabled={processing}
              className="w-full py-4 rounded-xl bg-gradient-to-r from-brand-cyan to-brand-mint text-slate-950 font-extrabold text-base flex items-center justify-center gap-2 shadow-xl shadow-brand-cyan/25 hover:opacity-95 transition-all"
            >
              {processing ? (
                <>
                  <span className="w-5 h-5 border-2 border-slate-950 border-t-transparent rounded-full animate-spin" />
                  <span>Procesando pago seguro...</span>
                </>
              ) : (
                <>
                  <Lock className="w-4 h-4" />
                  <span>{t.modals.checkout.payButton} ({planInfo.price})</span>
                </>
              )}
            </button>

            <p className="text-[11px] text-slate-400 text-center font-mono">
              {t.modals.checkout.secureNote}
            </p>

          </form>
        )}

      </div>
    </div>
  );
}
