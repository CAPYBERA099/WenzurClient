import { useState } from "react";
import { invoke } from "@tauri-apps/api/core";
import { Gamepad2, LogIn, Eye, EyeOff, AlertCircle } from "lucide-react";
import type { Account } from "../App";

interface Props {
  onLogin: (account: Account) => void;
}

export default function LoginPage({ onLogin }: Props) {
  const [username, setUsername] = useState("");
  const [password, setPassword] = useState("");
  const [showPassword, setShowPassword] = useState(false);
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState("");

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault();
    if (!username || !password) {
      setError("Введите логин и пароль");
      return;
    }

    setLoading(true);
    setError("");

    try {
      const account = await invoke<Account>("login_elyby", {
        username,
        password,
      });
      onLogin(account);
    } catch (err) {
      setError(String(err));
    } finally {
      setLoading(false);
    }
  };

  return (
    <div className="flex-1 flex items-center justify-center relative overflow-hidden">
      {/* Background effects */}
      <div className="absolute inset-0">
        <div className="absolute top-1/4 left-1/4 w-96 h-96 bg-wenz-accent/5 rounded-full blur-3xl" />
        <div className="absolute bottom-1/4 right-1/4 w-80 h-80 bg-purple-500/5 rounded-full blur-3xl" />
      </div>

      <div className="relative z-10 w-full max-w-md px-6">
        {/* Logo */}
        <div className="text-center mb-8">
          <div className="inline-flex items-center justify-center w-20 h-20 rounded-2xl bg-gradient-to-br from-wenz-accent/20 to-wenz-accent/5 border border-wenz-accent/30 mb-4">
            <Gamepad2 size={40} className="text-wenz-accent" />
          </div>
          <h1 className="text-3xl font-bold text-white">WenzLauncher</h1>
          <p className="text-gray-400 mt-2">Войдите через ely.by</p>
        </div>

        {/* Login form */}
        <form onSubmit={handleSubmit} className="space-y-4">
          <div className="bg-wenz-surface/80 backdrop-blur rounded-2xl border border-wenz-border p-6 space-y-4">
            {/* Error */}
            {error && (
              <div className="flex items-center gap-2 text-red-400 bg-red-500/10 rounded-lg px-4 py-3 text-sm">
                <AlertCircle size={16} />
                <span>{error}</span>
              </div>
            )}

            {/* Username / Email */}
            <div>
              <label className="block text-sm text-gray-400 mb-1.5">
                Логин или Email
              </label>
              <input
                type="text"
                value={username}
                onChange={(e) => setUsername(e.target.value)}
                className="w-full bg-wenz-bg/80 border border-wenz-border rounded-lg px-4 py-3 text-white placeholder-gray-500 focus:outline-none focus:border-wenz-accent/50 focus:ring-1 focus:ring-wenz-accent/30 transition-colors"
                placeholder="username или email@example.com"
                autoFocus
              />
            </div>

            {/* Password */}
            <div>
              <label className="block text-sm text-gray-400 mb-1.5">
                Пароль
              </label>
              <div className="relative">
                <input
                  type={showPassword ? "text" : "password"}
                  value={password}
                  onChange={(e) => setPassword(e.target.value)}
                  className="w-full bg-wenz-bg/80 border border-wenz-border rounded-lg px-4 py-3 pr-12 text-white placeholder-gray-500 focus:outline-none focus:border-wenz-accent/50 focus:ring-1 focus:ring-wenz-accent/30 transition-colors"
                  placeholder="••••••••"
                />
                <button
                  type="button"
                  onClick={() => setShowPassword(!showPassword)}
                  className="absolute right-3 top-1/2 -translate-y-1/2 text-gray-500 hover:text-gray-300 transition-colors"
                >
                  {showPassword ? <EyeOff size={18} /> : <Eye size={18} />}
                </button>
              </div>
            </div>

            {/* Submit */}
            <button
              type="submit"
              disabled={loading}
              className="w-full flex items-center justify-center gap-2 bg-wenz-accent hover:bg-wenz-accent/90 disabled:bg-wenz-accent/50 text-wenz-bg font-semibold rounded-lg px-4 py-3 transition-all duration-200 hover:shadow-lg hover:shadow-wenz-accent/20"
            >
              {loading ? (
                <>
                  <div className="w-5 h-5 border-2 border-wenz-bg/30 border-t-wenz-bg rounded-full animate-spin" />
                  <span>Входим...</span>
                </>
              ) : (
                <>
                  <LogIn size={18} />
                  <span>Войти</span>
                </>
              )}
            </button>
          </div>

          {/* Register link */}
          <p className="text-center text-sm text-gray-500">
            Нет аккаунта?{" "}
            <a
              href="https://ely.by/register"
              target="_blank"
              rel="noopener noreferrer"
              className="text-wenz-accent hover:text-wenz-accent/80 transition-colors"
            >
              Зарегистрируйтесь на ely.by
            </a>
          </p>
        </form>
      </div>
    </div>
  );
}
