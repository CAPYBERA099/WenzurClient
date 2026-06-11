import { useEffect, useRef, useState } from "react";
import { User } from "lucide-react";

interface Props {
  username: string;
  width?: number;
  height?: number;
  walk?: boolean;
}

export default function SkinViewer({
  username,
  width = 200,
  height = 300,
  walk = true,
}: Props) {
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const viewerRef = useRef<any>(null);
  const [error, setError] = useState(false);

  useEffect(() => {
    if (!canvasRef.current) return;

    let viewer: any = null;

    const initViewer = async () => {
      try {
        // Динамический импорт skinview3d
        const skinview3d = await import("skinview3d");

        viewer = new skinview3d.SkinViewer({
          canvas: canvasRef.current!,
          width,
          height,
          skin: `https://skinsystem.ely.by/skins/${username}.png`,
        });

        // Загружаем кейп (может не быть)
        viewer.loadCape(`https://skinsystem.ely.by/cloaks/${username}.png`).catch(() => {});

        // Камера
        viewer.camera.rotation.x = -0.1;
        viewer.camera.rotation.y = 0.5;
        viewer.zoom = 0.9;

        // Анимация
        if (walk) {
          viewer.animation = new skinview3d.WalkingAnimation();
          viewer.animation.speed = 0.6;
        } else {
          viewer.animation = new skinview3d.IdleAnimation();
        }

        // Управление
        viewer.autoRotate = false;
        viewer.controls.enableRotate = true;
        viewer.controls.enableZoom = true;
        viewer.controls.enablePan = false;

        // Прозрачный фон
        viewer.background = null;

        viewerRef.current = viewer;
        setError(false);
      } catch (err) {
        console.error("SkinViewer error:", err);
        setError(true);
      }
    };

    initViewer();

    return () => {
      if (viewer) {
        try {
          viewer.dispose();
        } catch {}
      }
    };
  }, [username, width, height, walk]);

  if (error) {
    return (
      <div
        className="flex flex-col items-center justify-center bg-wenz-surface rounded-xl border border-wenz-border"
        style={{ width, height }}
      >
        <User size={48} className="text-gray-600 mb-2" />
        <span className="text-gray-500 text-xs">3D скин</span>
      </div>
    );
  }

  return (
    <canvas
      ref={canvasRef}
      className="rounded-lg"
      style={{ width, height }}
    />
  );
}
