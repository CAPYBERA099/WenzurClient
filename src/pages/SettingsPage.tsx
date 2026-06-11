import { useState, useEffect } from "react";
import { invoke } from "@tauri-apps/api/core";
import {
  Settings,
  Save,
  RotateCcw,
  Cpu,
  HardDrive,
  Monitor,
  Loader2,
  Check,
  FolderOpen,
} from "lucide-react";

interface LauncherConfig {
  ram_mb: number;
  java_path: string;
  game_dir: string;
  resolution_width: number;
  resolution_height: number;
  jvm_args: string;
}

export default function SettingsPage() {
  const [config, setConfig] = useState<LauncherConfig | null>(null);
  const [loading, setLoading] = useState(true);
  const [saving, setSaving] = useState(false);
  const [saved, setSaved] = useState(false);
  const [javaInfo, setJavaInfo] = useState("");

  useEffect(() => {
    loadConfig();
  }, []);

  const loadConfig = async () => {
    try {
      const [cfg, java] = await Promise.all([
        invoke<LauncherConfig>("get_config"),
        invoke<string>("get_java_info"),
      ]);
      setConfig(cfg);
      setJavaInfo(java);
    } catch (err) {
      console.error(err);
    } finally {
      setLoading(false);
    }
  };

  const handleSave = async () => {
    if (!config) return;
    setSaving(true);
    try {
      await invoke("save_config", { config });
      setSaved(true);
      setTimeout(() => setSaved(false), 2000);
    } catch (err) {
      alert(String(err));
    } finally {
      setSaving(false);
    }
  };

  const handleReset = () => {
    setConfig({
      ram_mb: 4096,
      java_path: javaInfo === "Java не найдена" ? "java" : javaInfo,
      game_dir: "",
      resolution_width: 1920,
      resolution_height: 1080,
      jvm_args: "-XX:+UseG1GC -XX:+UnlockExperimentalVMOptions -XX:G1NewSizePercent=20 -XX:G1ReservePercent=20",
    });
  };

  const updateConfig = (key: keyof LauncherConfig, value: any) => {
    if (!config) return;
    setConfig({ ...config, [key]: value });
  };

  if (loading || !config) {
    return (
      <div className="h-full flex items-center justify-center">
        <Loader2 size={32} className="text-wenz-accent animate-spin" />
      </div>
    );
  }

  return (
    <div className="p-6 space-y-6 max-w-2xl">
      <div className="flex items-center justify-between">
        <div>
          <h1 className="text-2xl font-bold text-white flex items-center gap-2">
            <Settings size={24} className="text-wenz-accent" />
            Настройки
          </h1>
          <p className="text-gray-400 text-sm mt-1">Конфигурация лаунчера и игры</p>
        </div>
        <div className="flex gap-2">
          <button
            onClick={handleReset}
            className="flex items-center gap-1.5 bg-wenz-surface border border-wenz-border rounded-lg px-3 py-2 text-sm text-gray-400 hover:text-white transition-colors"
          >
            <RotateCcw size={14} />
            Сброс
          </button>
          <button
            onClick={handleSave}
            disabled={saving}
            className="flex items-center gap-1.5 bg-wenz-accent hover:bg-wenz-accent/90 text-wenz-bg rounded-lg px-4 py-2 text-sm font-medium transition-colors"
          >
            {saving ? (
              <Loader2 size={14} className="animate-spin" />
            ) : saved ? (
              <Check size={14} />
            ) : (
              <Save size={14} />
            )}
            {saved ? "Сохранено!" : "Сохранить"}
          </button>
        </div>
      </div>

      {/* RAM */}
      <div className="bg-wenz-surface border border-wenz-border rounded-2xl p-6">
        <h3 className="text-lg font-semibold text-white mb-4 flex items-center gap-2">
          <Cpu size={18} className="text-wenz-accent" />
          Производительность
        </h3>
        <div className="space-y-4">
          <div>
            <div className="flex justify-between mb-2">
              <label className="text-sm text-gray-400">Оперативная память (RAM)</label>
              <span className="text-sm text-wenz-accent font-medium">{config.ram_mb} MB</span>
            </div>
            <input
              type="range"
              min={1024}
              max={16384}
              step={512}
              value={config.ram_mb}
              onChange={(e) => updateConfig("ram_mb", parseInt(e.target.value))}
              className="w-full h-2 bg-wenz-bg rounded-lg appearance-none cursor-pointer accent-wenz-accent"
            />
            <div className="flex justify-between text-xs text-gray-600 mt-1">
              <span>1 GB</span>
              <span>4 GB</span>
              <span>8 GB</span>
              <span>16 GB</span>
            </div>
          </div>

          <div>
            <label className="text-sm text-gray-400 block mb-2">JVM аргументы</label>
            <input
              type="text"
              value={config.jvm_args}
              onChange={(e) => updateConfig("jvm_args", e.target.value)}
              className="w-full bg-wenz-bg border border-wenz-border rounded-lg px-4 py-2.5 text-white text-sm font-mono focus:outline-none focus:border-wenz-accent/50"
            />
          </div>
        </div>
      </div>

      {/* Java */}
      <div className="bg-wenz-surface border border-wenz-border rounded-2xl p-6">
        <h3 className="text-lg font-semibold text-white mb-4 flex items-center gap-2">
          <HardDrive size={18} className="text-wenz-accent" />
          Java и пути
        </h3>
        <div className="space-y-4">
          <div>
            <label className="text-sm text-gray-400 block mb-2">Путь к Java</label>
            <input
              type="text"
              value={config.java_path}
              onChange={(e) => updateConfig("java_path", e.target.value)}
              className="w-full bg-wenz-bg border border-wenz-border rounded-lg px-4 py-2.5 text-white text-sm font-mono focus:outline-none focus:border-wenz-accent/50"
              placeholder="java"
            />
            <p className="text-xs text-gray-600 mt-1">
              Найдено: {javaInfo}
            </p>
          </div>

          <div>
            <label className="text-sm text-gray-400 block mb-2 flex items-center gap-1">
              <FolderOpen size={14} />
              Директория игры
            </label>
            <input
              type="text"
              value={config.game_dir}
              onChange={(e) => updateConfig("game_dir", e.target.value)}
              className="w-full bg-wenz-bg border border-wenz-border rounded-lg px-4 py-2.5 text-white text-sm font-mono focus:outline-none focus:border-wenz-accent/50"
              placeholder="По умолчанию"
            />
          </div>
        </div>
      </div>

      {/* Resolution */}
      <div className="bg-wenz-surface border border-wenz-border rounded-2xl p-6">
        <h3 className="text-lg font-semibold text-white mb-4 flex items-center gap-2">
          <Monitor size={18} className="text-wenz-accent" />
          Разрешение окна
        </h3>
        <div className="grid grid-cols-2 gap-4">
          <div>
            <label className="text-sm text-gray-400 block mb-2">Ширина</label>
            <input
              type="number"
              value={config.resolution_width}
              onChange={(e) => updateConfig("resolution_width", parseInt(e.target.value) || 1920)}
              className="w-full bg-wenz-bg border border-wenz-border rounded-lg px-4 py-2.5 text-white text-sm focus:outline-none focus:border-wenz-accent/50"
            />
          </div>
          <div>
            <label className="text-sm text-gray-400 block mb-2">Высота</label>
            <input
              type="number"
              value={config.resolution_height}
              onChange={(e) => updateConfig("resolution_height", parseInt(e.target.value) || 1080)}
              className="w-full bg-wenz-bg border border-wenz-border rounded-lg px-4 py-2.5 text-white text-sm focus:outline-none focus:border-wenz-accent/50"
            />
          </div>
        </div>
        <div className="flex gap-2 mt-3">
          {[
            [1920, 1080, "1080p"],
            [2560, 1440, "1440p"],
            [1280, 720, "720p"],
          ].map(([w, h, label]) => (
            <button
              key={String(label)}
              onClick={() => {
                updateConfig("resolution_width", w);
                updateConfig("resolution_height", h);
              }}
              className="text-xs bg-wenz-bg border border-wenz-border rounded-lg px-3 py-1.5 text-gray-400 hover:text-white hover:border-wenz-accent/30 transition-colors"
            >
              {String(label)}
            </button>
          ))}
        </div>
      </div>
    </div>
  );
}
