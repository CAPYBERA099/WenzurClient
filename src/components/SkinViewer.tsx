import { useEffect, useRef } from "react";
import * as skinview3d from "skinview3d";

interface Props {
  username: string;
  width?: number;
  height?: number;
  walk?: boolean;
}

export default function SkinViewer({ username, width = 200, height = 300, walk = true }: Props) {
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const viewerRef = useRef<skinview3d.SkinViewer | null>(null);

  useEffect(() => {
    if (!canvasRef.current) return;

    // Создаём 3D viewer
    const viewer = new skinview3d.SkinViewer({
      canvas: canvasRef.current,
      width,
      height,
      skin: `https://skinsystem.ely.by/skins/${username}.png`,
    });

    // Загружаем кейп
    viewer.loadCape(`https://skinsystem.ely.by/cloaks/${username}.png`).catch(() => {
      // Кейпа может не быть — игнорируем
    });

    // Настройки камеры
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

    // Автоповорот
    viewer.autoRotate = false;
    viewer.controls.enableRotate = true;
    viewer.controls.enableZoom = true;
    viewer.controls.enablePan = false;

    // Фон прозрачный
    viewer.background = null;

    viewerRef.current = viewer;

    return () => {
      viewer.dispose();
    };
  }, [username, width, height, walk]);

  return (
    <canvas
      ref={canvasRef}
      className="rounded-lg"
      style={{ width, height }}
    />
  );
}
