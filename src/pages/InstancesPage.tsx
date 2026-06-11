import { useState, useEffect } from "react";
import { invoke } from "@tauri-apps/api/core";
import { listen } from "@tauri-apps/api/event";
import {
  Download,
  Trash2,
  Play,
  Loader2,
  Check,
  Search,
  Package,
  AlertCircle,
} from "lucide-react";
import type { Account } from "../App";

interface GameVersion {
  id: string;
  version_type: string;
  url: string;
  release_time: string;
}

interface DownloadProgress {
  stage: string;
  progress: number;
  detail: string;
}

interface Props {
  account: Account;
}

export default function InstancesPage({ account }: Props) {
  const [versions, setVersions] = useState<GameVersion[]>([]);
  const [installedVersions, setInstalledVersions] = useState<string[]>([]);
  const [loading, setLoading] = useState(true);
  const [search, setSearch] = useState("");
  const [filter, setFilter] = useState<"all" | "installed" | "release" | "snapshot">("all");
  const [installingId, setInstallingId] = useState<string | null>(null);
  const [progress, setProgress] = useState<DownloadProgress | null>(null);
  const [error, setError] = useState("");

  useEffect(() => {
    loadData();
    const unlisten = listen<DownloadProgress>("download-progress", (e) => {
      setProgress(e.payload);
    });
    return () => { unlisten.then((fn) => fn()); };
  }, []);

  const loadData = async () => {
    try {
      const [v, installed] = await Promise.all([
        invoke<GameVersion[]>("fetch_versions"),
        invoke<string[]>("get_installed_versions"),
      ]);
      setVersions(v);
      setInstalledVersions(installed);
    } catch (err) {
      setError(String(err));
    } finally {
      setLoading(false);
    }
  };

  const handleInstall = async (version: GameVersion) => {
    setInstallingId(version.id);
    setProgress({ stage: "Начало", progress: 0, detail: "" });
    try {
      await invoke("install_version", {
        versionId: version.id,
        versionUrl: version.url,
      });
      const installed = await invoke<string[]>("get_installed_versions");
      setInstalledVersions(installed);
    } catch (err) {
      setError(String(err));
    } finally {
      setInstallingId(null);
      setTimeout(() => setProgress(null), 2000);
    }
  };

  const handleDelete = async (versionId: string) => {
    try {
      await invoke("delete_version", { versionId });
      const installed = await invoke<string[]>("get_installed_versions");
      setInstalledVersions(installed);
    } catch (err) {
      setError(String(err));
    }
  };

  const handleLaunch = async (versionId: string) => {
    try {
      const config = await invoke<any>("get_config");
      await invoke("launch_game", {
        versionId,
        username: account.username,
        uuid: account.uuid,
        accessToken: account.access_token,
        ramMb: config.ram_mb || 4096,
        javaPath: config.java_path || "java",
      });
    } catch (err) {
      alert(String(err));
    }
  };

  const filtered = versions.filter((v) => {
    if (search && !v.id.toLowerCase().includes(search.toLowerCase())) return false;
    if (filter === "installed") return installedVersions.includes(v.id);
    if (filter === "release") return v.version_type === "release";
    if (filter === "snapshot") return v.version_type === "snapshot";
    return true;
  });

  if (loading) {
    return (
      <div className="h-full flex items-center justify-center">
        <Loader2 size={32} className="text-wenz-accent animate-spin" />
      </div>
    );
  }

  return (
    <div className="p-6 space-y-6">
      <div>
        <h1 className="text-2xl font-bold text-white">Версии Minecraft</h1>
        <p className="text-gray-400 text-sm mt-1">
          Установлено: {installedVersions.length} | Доступно: {versions.length}
        </p>
      </div>

      {error && (
        <div className="flex items-center gap-2 bg-red-500/10 border border-red-500/20 rounded-xl px-4 py-3 text-red-400 text-sm">
          <AlertCircle size={16} />
          {error}
          <button onClick={() => setError("")} className="ml-auto text-red-400/60 hover:text-red-400">✕</button>
        </div>
      )}

      {/* Search + filter */}
      <div className="flex gap-3">
        <div className="flex-1 relative">
          <Search size={16} className="absolute left-3 top-1/2 -translate-y-1/2 text-gray-500" />
          <input
            type="text"
            value={search}
            onChange={(e) => setSearch(e.target.value)}
            placeholder="Найти версию..."
            className="w-full bg-wenz-surface border border-wenz-border rounded-lg pl-10 pr-4 py-2.5 text-white text-sm placeholder-gray-500 focus:outline-none focus:border-wenz-accent/50"
          />
        </div>
        <div className="flex bg-wenz-surface border border-wenz-border rounded-lg overflow-hidden">
          {(["all", "installed", "release", "snapshot"] as const).map((f) => (
            <button
              key={f}
              onClick={() => setFilter(f)}
              className={`px-3 py-2 text-xs font-medium transition-colors ${
                filter === f
                  ? "bg-wenz-accent/20 text-wenz-accent"
                  : "text-gray-400 hover:text-white"
              }`}
            >
              {f === "all" ? "Все" : f === "installed" ? "Установленные" : f === "release" ? "Релизы" : "Снапшоты"}
            </button>
          ))}
        </div>
      </div>

      {/* Progress */}
      {installingId && progress && (
        <div className="bg-wenz-surface border border-wenz-accent/20 rounded-xl p-4">
          <div className="flex justify-between text-sm mb-2">
            <span className="text-white">
              Установка {installingId}: {progress.stage}
            </span>
            <span className="text-wenz-accent">{Math.round(progress.progress * 100)}%</span>
          </div>
          <div className="w-full bg-wenz-bg rounded-full h-2 overflow-hidden">
            <div
              className="bg-wenz-accent h-full rounded-full transition-all duration-300"
              style={{ width: `${progress.progress * 100}%` }}
            />
          </div>
          <p className="text-xs text-gray-500 mt-2 truncate">{progress.detail}</p>
        </div>
      )}

      {/* Version list */}
      <div className="space-y-2">
        {filtered.length === 0 ? (
          <div className="text-center py-12 text-gray-500">
            <Package size={40} className="mx-auto mb-3 opacity-50" />
            <p>Версии не найдены</p>
          </div>
        ) : (
          filtered.map((v) => {
            const installed = installedVersions.includes(v.id);
            const isInstalling = installingId === v.id;

            return (
              <div
                key={v.id}
                className="flex items-center justify-between bg-wenz-surface border border-wenz-border rounded-xl px-4 py-3 hover:border-wenz-accent/20 transition-colors group"
              >
                <div className="flex items-center gap-3">
                  <div
                    className={`w-2 h-2 rounded-full ${
                      installed ? "bg-wenz-accent" : "bg-gray-600"
                    }`}
                  />
                  <span className="text-white font-medium">{v.id}</span>
                  <span
                    className={`text-xs px-2 py-0.5 rounded-full ${
                      v.version_type === "release"
                        ? "bg-green-500/10 text-green-400"
                        : "bg-yellow-500/10 text-yellow-400"
                    }`}
                  >
                    {v.version_type}
                  </span>
                  {installed && (
                    <span className="text-xs text-wenz-accent flex items-center gap-1">
                      <Check size={12} /> установлена
                    </span>
                  )}
                </div>

                <div className="flex items-center gap-2 opacity-0 group-hover:opacity-100 transition-opacity">
                  {installed ? (
                    <>
                      <button
                        onClick={() => handleLaunch(v.id)}
                        className="flex items-center gap-1.5 bg-wenz-accent/20 hover:bg-wenz-accent/30 text-wenz-accent rounded-lg px-3 py-1.5 text-sm transition-colors"
                      >
                        <Play size={14} /> Играть
                      </button>
                      <button
                        onClick={() => handleDelete(v.id)}
                        className="p-1.5 text-gray-500 hover:text-red-400 transition-colors"
                      >
                        <Trash2 size={14} />
                      </button>
                    </>
                  ) : isInstalling ? (
                    <Loader2 size={16} className="text-wenz-accent animate-spin" />
                  ) : (
                    <button
                      onClick={() => handleInstall(v)}
                      disabled={!!installingId}
                      className="flex items-center gap-1.5 bg-blue-500/20 hover:bg-blue-500/30 text-blue-400 rounded-lg px-3 py-1.5 text-sm transition-colors disabled:opacity-50"
                    >
                      <Download size={14} /> Установить
                    </button>
                  )}
                </div>
              </div>
            );
          })
        )}
      </div>
    </div>
  );
}
