import { useState } from "react";
import { User, RotateCcw, ExternalLink } from "lucide-react";
import type { Account } from "../App";
import SkinViewer from "../components/SkinViewer";

interface Props {
  account: Account;
}

export default function SkinsPage({ account }: Props) {
  const [walkAnimation, setWalkAnimation] = useState(true);

  const skinUrl = `https://skinsystem.ely.by/skins/${account.username}.png`;
  const capeUrl = `https://skinsystem.ely.by/cloaks/${account.username}.png`;

  return (
    <div className="p-6 space-y-6">
      <div>
        <h1 className="text-2xl font-bold text-white">Скин и косметика</h1>
        <p className="text-gray-400 text-sm mt-1">
          Скины подгружаются с вашего аккаунта ely.by
        </p>
      </div>

      <div className="grid grid-cols-1 lg:grid-cols-2 gap-6">
        {/* 3D Skin Viewer */}
        <div className="bg-wenz-surface border border-wenz-border rounded-2xl p-6 flex flex-col items-center">
          <h3 className="text-lg font-semibold text-white mb-4">3D Просмотр</h3>

          <div className="w-[300px] h-[400px] mb-4">
            <SkinViewer
              username={account.username}
              width={300}
              height={400}
              walk={walkAnimation}
            />
          </div>

          <div className="flex items-center gap-4">
            <button
              onClick={() => setWalkAnimation(!walkAnimation)}
              className="flex items-center gap-2 bg-wenz-bg border border-wenz-border rounded-lg px-4 py-2 text-sm text-gray-300 hover:text-white hover:border-wenz-accent/30 transition-colors"
            >
              <RotateCcw size={14} />
              {walkAnimation ? "Остановить" : "Анимация ходьбы"}
            </button>
          </div>

          <p className="text-xs text-gray-500 mt-3">
            Крутите мышкой для поворота, колёсиком для зума
          </p>
        </div>

        {/* Info + links */}
        <div className="space-y-4">
          {/* Account info */}
          <div className="bg-wenz-surface border border-wenz-border rounded-2xl p-6">
            <h3 className="text-lg font-semibold text-white mb-4 flex items-center gap-2">
              <User size={18} className="text-wenz-accent" />
              Аккаунт
            </h3>
            <div className="space-y-3">
              <div className="flex justify-between items-center">
                <span className="text-gray-400 text-sm">Никнейм</span>
                <span className="text-white font-medium">{account.username}</span>
              </div>
              <div className="flex justify-between items-center">
                <span className="text-gray-400 text-sm">UUID</span>
                <span className="text-gray-300 text-xs font-mono">{account.uuid}</span>
              </div>
            </div>
          </div>

          {/* Skin preview 2D */}
          <div className="bg-wenz-surface border border-wenz-border rounded-2xl p-6">
            <h3 className="text-lg font-semibold text-white mb-4">Текстура скина</h3>
            <div className="flex justify-center bg-wenz-bg rounded-xl p-4 mb-4"
              style={{ backgroundImage: "url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAIAAAACCAYAAABytg0kAAAAEklEQVQIW2P4z8DwHwMDEwMACmgCARfnFd4AAAAASUVORK5CYII=\")", backgroundRepeat: "repeat" }}
            >
              <img
                src={skinUrl}
                alt="Skin texture"
                className="image-rendering-pixelated max-w-[256px]"
                style={{ imageRendering: "pixelated" }}
                onError={(e) => {
                  (e.target as HTMLImageElement).src = "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNk+M9QDwADhgGAWjR9awAAAABJRU5ErkJggg==";
                }}
              />
            </div>
            <p className="text-xs text-gray-500 text-center">
              Оригинальная текстура с ely.by
            </p>
          </div>

          {/* Cape */}
          <div className="bg-wenz-surface border border-wenz-border rounded-2xl p-6">
            <h3 className="text-lg font-semibold text-white mb-4">Плащ</h3>
            <div className="flex justify-center bg-wenz-bg rounded-xl p-4 mb-4">
              <img
                src={capeUrl}
                alt="Cape"
                className="max-w-[128px]"
                style={{ imageRendering: "pixelated" }}
                onError={(e) => {
                  const parent = (e.target as HTMLImageElement).parentElement;
                  if (parent) {
                    parent.innerHTML = '<p class="text-gray-500 text-sm py-4">Плащ не установлен</p>';
                  }
                }}
              />
            </div>
          </div>

          {/* Change skin */}
          <a
            href={`https://ely.by/skins`}
            target="_blank"
            rel="noopener noreferrer"
            className="flex items-center justify-center gap-2 w-full bg-wenz-accent/10 hover:bg-wenz-accent/20 border border-wenz-accent/20 text-wenz-accent rounded-xl px-4 py-3 text-sm font-medium transition-colors"
          >
            <ExternalLink size={16} />
            Сменить скин на ely.by
          </a>
        </div>
      </div>
    </div>
  );
}
