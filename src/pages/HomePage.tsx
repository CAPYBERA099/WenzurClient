import { useState, useEffect, useRef } from "react";
import { invoke } from "@tauri-apps/api/core";
import { listen } from "@tauri-apps/api/event";
import { Play, Download, ChevronDown, Check, Loader2 } from "lucide-react";
import type { Account } from "../App";
import SkinViewer from "../components/SkinViewer";

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

export default function HomePage({ account }: Props) {
  const [versions, setVersions] = useState<GameVersion[]>([]);
  const [installedVersions, setInstalledVersions] = useState<string[]>([]);
  const [selectedVersion, setSelectedVersion] = useState<GameVersion | null>(null);
  const [showVersions, setShowVersions] = useState(false);
  const [showSnapshots, setShowSnapshots] = useState(false);
  const [loadingVersions, setLoadingVersions] = useState(true);
  const [installing, setInstalling] = useState(false);
  const [launching, setLaunching] = useState(false);
  const [progress, setProgress] = useState<DownloadProgress | null>(null);
  const dropdownRef = useRef<HTMLDivElement>(null);

  useEffect(() => {
    loadVersions();
    loadInstalled();

    const unlisten = listen<DownloadProgress>("download-progress", (event) => {
      setProgress(event.payload);
    });

    return () => {
      unlisten.then((fn) => fn());
    };
  }, []);

  // Close dropdown on outside click
  useEffect(() => {
    const handler = (e: MouseEvent) => {
      if (dropdownRef.current && !dropdownRef.current.contains(e.target as Node)) {
        setShowVersions(false);
      }
    };
    document.addEventListener("mousedown", handler);
    return () => document.removeEventListener("mousedown", handler);
  }, []);

  const loadVersions = async () => {
    try {
      const v = await invoke<GameVersion[]>("fetch_versions");
      setVersions(v);
      // По умолчанию выбираем первый релиз
      const firstRelease = v.find((ver) => ver.version_type === "release");
      if (firstRelease) setSelectedVersion(firstRelease);
    } catch (err) {
      console.error("Ошибка загрузки версий:", err);
    } finally {
      setLoadingVersions(false);
    }
  };

  const loadInstalled = async () => {
    try {
      const installed = await invoke<string[]>("get_installed_versions");
      setInstalledVersions(installed);
    } catch (err) {
      console.error(err);
    }
  };

  const isInstalled = selectedVersion ? installedVersions.includes(selectedVersion.id) : false;

  const handleInstall = async () => {
    if (!selectedVersion) return;
    setInstalling(true);
    setProgress({ stage: "Начало", progress: 0, detail: "" });
    try {
      await invoke("install_version", {
        versionId: selectedVersion.id,
        versionUrl: selectedVersion.url,
      });
      await loadInstalled();
    } catch (err) {
      console.error("Ошибка установки:", err);
      setProgress({ stage: "Ошибка", progress: 0, detail: String(err) });
    } finally {
      setInstalling(false);
      setTimeout(() => setProgress(null), 3000);
    }
  };

  const handleLaunch = async () => {
    if (!selectedVersion || !account) return;
    
    if (!isInstalled) {
      await handleInstall();
      return;
    }

    setLaunching(true);
    try {
      const config = await invoke<any>("get_config");
      await invoke("launch_game", {
        versionId: selectedVersion.id,
        username: account.username,
        uuid: account.uuid,
        accessToken: account.access_token,
        ramMb: config.ram_mb || 4096,
        javaPath: config.java_path || "java",
      });
    } catch (err) {
      console.error("Ошибка запуска:", err);
      alert(String(err));
    } finally {
      setLaunching(false);
    }
  };

  const filteredVersions = versions.filter(
    (v) => showSnapshots || v.version_type === "release"
  );

  return (
    <div className="h-full flex flex-col items-center justify-center relative p-8">
      {/* Background glow */}
      <div className="absolute inset-0 pointer-events-none">
        <div className="absolute top-1/3 left-1/2 -translate-x-1/2 w-[600px] h-[600px] bg-wenz-accent/5 rounded-full blur-[120px]" />
      </div>

      <div className="relative z-10 flex flex-col items-center gap-8 w-full max-w-lg">
        {/* 3D Skin Viewer */}
        <div className="w-48 h-64">
          <SkinViewer username={account.username} width={192} height={256} />
        </div>

        {/* Player name */}
        <div className="text-center">
          <h2 className="text-2xl font-bold text-white">{account.username}</h2>
          <p className="text-gray-400 text-sm mt-1">ely.by</p>
        </div>

        {/* Version selector */}
        <div className="w-full max-w-sm" ref={dropdownRef}>
          <div className="relative">
            <button
              onClick={() => setShowVersions(!showVersions)}
              disabled={loadingVersions}
              className="w-full flex items-center justify-between bg-wenz-surface border border-wenz-border rounded-xl px-4 py-3 text-white hover:border-wenz-accent/30 transition-colors"
            >
              <div className="flex items-center gap-3">
                {loadingVersions ? (
                  <Loader2 size={16} className="animate-spin text-gray-400" />
                ) : selectedVersion ? (
                  <>
                    <span className="font-medium">{selectedVersion.id}</span>
                    {isInstalled && (
                      <span className="text-xs bg-wenz-accent/20 text-wenz-accent px-2 py-0.5 rounded-full">
                        установлена
                      </span>
                    )}
                    <span className="text-xs text-gray-500">
                      {selectedVersion.version_type}
                    </span>
                  </>
                ) : (
                  <span className="text-gray-400">Выберите версию</span>
                )}
              </div>
              <ChevronDown
                size={16}
                className={`text-gray-400 transition-transform ${
                  showVersions ? "rotate-180" : ""
                }`}
              />
            </button>

            {/* Dropdown */}
            {showVersions && (
              <div className="absolute top-full left-0 right-0 mt-2 bg-wenz-surface border border-wenz-border rounded-xl overflow-hidden shadow-2xl z-50">
                {/* Snapshot toggle */}
                <div className="px-4 py-2 border-b border-wenz-border flex items-center justify-between">
                  <span className="text-sm text-gray-400">Снапшоты</span>
                  <button
                    onClick={() => setShowSnapshots(!showSnapshots)}
                    className={`w-10 h-5 rounded-full transition-colors ${
                      showSnapshots ? "bg-wenz-accent" : "bg-gray-600"
                    }`}
                  >
                    <div
                      className={`w-4 h-4 rounded-full bg-white transition-transform ${
                        showSnapshots ? "translate-x-5" : "translate-x-0.5"
                      }`}
                    />
                  </button>
                </div>
                <div className="max-h-64 overflow-y-auto scrollbar-thin">
                  {filteredVersions.map((v) => {
                    const installed = installedVersions.includes(v.id);
                    return (
                      <button
                        key={v.id}
                        onClick={() => {
                          setSelectedVersion(v);
                          setShowVersions(false);
                        }}
                        className={`w-full flex items-center justify-between px-4 py-2.5 hover:bg-wenz-accent/10 transition-colors text-left ${
                          selectedVersion?.id === v.id ? "bg-wenz-accent/5" : ""
                        }`}
                      >
                        <div className="flex items-center gap-2">
                          <span className="text-white text-sm">{v.id}</span>
                          <span className="text-xs text-gray-500">{v.version_type}</span>
                        </div>
                        {installed && <Check size={14} className="text-wenz-accent" />}
                      </button>
                    );
                  })}
                </div>
              </div>
            )}
          </div>
        </div>

        {/* Progress bar */}
        {progress && installing && (
          <div className="w-full max-w-sm">
            <div className="bg-wenz-surface border border-wenz-border rounded-xl p-4">
              <div className="flex justify-between text-sm mb-2">
                <span className="text-white">{progress.stage}</span>
                <span className="text-gray-400">
                  {Math.round(progress.progress * 100)}%
                </span>
              </div>
              <div className="w-full bg-wenz-bg rounded-full h-2 overflow-hidden">
                <div
                  className="bg-wenz-accent h-full rounded-full transition-all duration-300"
                  style={{ width: `${progress.progress * 100}%` }}
                />
              </div>
              <p className="text-xs text-gray-500 mt-2 truncate">{progress.detail}</p>
            </div>
          </div>
        )}

        {/* Play / Install button */}
        <button
          onClick={handleLaunch}
          disabled={!selectedVersion || installing || launching}
          className={`w-full max-w-sm py-4 rounded-xl font-bold text-lg transition-all duration-300 flex items-center justify-center gap-3 ${
            installing
              ? "bg-yellow-500/20 text-yellow-400 border border-yellow-500/30 cursor-wait"
              : launching
              ? "bg-wenz-accent/20 text-wenz-accent border border-wenz-accent/30 cursor-wait"
              : isInstalled
              ? "bg-wenz-accent hover:bg-wenz-accent/90 text-wenz-bg hover:shadow-lg hover:shadow-wenz-accent/25 hover:scale-[1.02] active:scale-[0.98]"
              : "bg-blue-500 hover:bg-blue-600 text-white hover:shadow-lg hover:shadow-blue-500/25 hover:scale-[1.02] active:scale-[0.98]"
          }`}
        >
          {installing ? (
            <>
              <Loader2 size={22} className="animate-spin" />
              Устанавливается...
            </>
          ) : launching ? (
            <>
              <Loader2 size={22} className="animate-spin" />
              Запуск...
            </>
          ) : isInstalled ? (
            <>
              <Play size={22} />
              ИГРАТЬ
            </>
          ) : (
            <>
              <Download size={22} />
              УСТАНОВИТЬ И ИГРАТЬ
            </>
          )}
        </button>
      </div>
    </div>
  );
}
