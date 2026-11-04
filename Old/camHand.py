import cv2
import mediapipe as mp
import numpy as np

# Initialisation de MediaPipe Hands
mp_hands = mp.solutions.hands
mp_drawing = mp.solutions.drawing_utils
mp_drawing_styles = mp.solutions.drawing_styles

# Paramètres de dessin pour les landmarks
hand_connections = mp_hands.HAND_CONNECTIONS
landmark_drawing_spec = mp_drawing.DrawingSpec(thickness=2, circle_radius=2, color=(0, 255, 0))
connection_drawing_spec = mp_drawing.DrawingSpec(thickness=2, circle_radius=2, color=(255, 0, 0))

def draw_hand_info(image, hand_landmarks, hand_label):
    """Dessine les landmarks et affiche les positions des mains"""
    h, w, _ = image.shape
    
    # Dessiner les landmarks et connections
    mp_drawing.draw_landmarks(
        image,
        hand_landmarks,
        hand_connections,
        landmark_drawing_spec,
        connection_drawing_spec)
    
    # Récupérer les points clés
    landmarks = hand_landmarks.landmark
    
    # Point de référence - Paume (point 0)
    palm = landmarks[0]
    palm_x, palm_y = int(palm.x * w), int(palm.y * h)
    
    # Poignet (point 0) pour l'orientation
    wrist = landmarks[0]
    
    # Bout de l'index (point 8)
    index_tip = landmarks[8]
    index_x, index_y = int(index_tip.x * w), int(index_tip.y * h)
    
    # Bout du pouce (point 4)
    thumb_tip = landmarks[4]
    thumb_x, thumb_y = int(thumb_tip.x * w), int(thumb_tip.y * h)
    
    # Calcul de la position relative (normalisée)
    rel_x = palm.x  # 0.0 (gauche) à 1.0 (droite)
    rel_y = palm.y  # 0.0 (haut) à 1.0 (bas)
    
    # Déterminer la zone de l'écran
    if rel_x < 0.33:
        horizontal_pos = "GAUCHE"
        color_h = (0, 0, 255)  # Rouge
    elif rel_x < 0.66:
        horizontal_pos = "CENTRE"
        color_h = (0, 255, 255)  # Jaune
    else:
        horizontal_pos = "DROITE"
        color_h = (255, 0, 0)  # Bleu
    
    if rel_y < 0.33:
        vertical_pos = "HAUT"
        color_v = (0, 255, 0)  # Vert
    elif rel_y < 0.66:
        vertical_pos = "MI-HAUTEUR"
        color_v = (0, 255, 255)  # Jaune
    else:
        vertical_pos = "BAS"
        color_v = (0, 165, 255)  # Orange
    
    # Afficher les informations de position
    cv2.putText(image, f"MAIN: {hand_label}", (10, 30), 
                cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255, 255, 255), 2)
    cv2.putText(image, f"Position: {horizontal_pos} - {vertical_pos}", (10, 60), 
                cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 255), 2)
    cv2.putText(image, f"Coords: X={rel_x:.2f}, Y={rel_y:.2f}", (10, 90), 
                cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 255), 2)
    
    # Dessiner un cercle sur la paume
    cv2.circle(image, (palm_x, palm_y), 10, (0, 255, 255), -1)
    
    # Dessiner une ligne d'orientation main
    cv2.line(image, (palm_x, palm_y), (index_x, index_y), (255, 255, 0), 2)
    
    # Afficher la zone horizontale
    cv2.putText(image, horizontal_pos, (palm_x + 15, palm_y - 10), 
                cv2.FONT_HERSHEY_SIMPLEX, 0.6, color_h, 2)
    
    # Afficher la zone verticale
    cv2.putText(image, vertical_pos, (palm_x + 15, palm_y + 15), 
                cv2.FONT_HERSHEY_SIMPLEX, 0.6, color_v, 2)
    
    return rel_x, rel_y

def main():
    # Initialiser le détecteur de mains
    with mp_hands.Hands(
        model_complexity=0,
        min_detection_confidence=0.5,
        min_tracking_confidence=0.5,
        max_num_hands=2) as hands:
        
        # Démarrer la capture vidéo
        cap = cv2.VideoCapture(0)
        
        # Vérifier si la caméra est ouverte
        if not cap.isOpened():
            print("Erreur: Impossible d'ouvrir la caméra")
            return
        
        print("Démarrage du tracking des mains...")
        print("Appuyez sur 'q' pour quitter")
        print("Appuyez sur 'r' pour réinitialiser les compteurs")
        
        while True:
            # Lire une frame de la caméra
            success, image = cap.read()
            if not success:
                print("Erreur: Impossible de lire la frame")
                break
            
            # Inverser l'image horizontalement pour un effet miroir
            image = cv2.flip(image, 1)
            
            # Convertir BGR en RGB
            image_rgb = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
            
            # Traiter l'image et détecter les mains
            results = hands.process(image_rgb)
            
            # Dessiner un quadrillage de référence
            h, w, _ = image.shape
            cv2.line(image, (w//3, 0), (w//3, h), (50, 50, 50), 1)
            cv2.line(image, (2*w//3, 0), (2*w//3, h), (50, 50, 50), 1)
            cv2.line(image, (0, h//3), (w, h//3), (50, 50, 50), 1)
            cv2.line(image, (0, 2*h//3), (w, 2*h//3), (50, 50, 50), 1)
            
            # Afficher les labels des zones
            cv2.putText(image, "GAUCHE", (w//6, 30), 
                       cv2.FONT_HERSHEY_SIMPLEX, 0.6, (100, 100, 100), 1)
            cv2.putText(image, "CENTRE", (w//2-30, 30), 
                       cv2.FONT_HERSHEY_SIMPLEX, 0.6, (100, 100, 100), 1)
            cv2.putText(image, "DROITE", (5*w//6-30, 30), 
                       cv2.FONT_HERSHEY_SIMPLEX, 0.6, (100, 100, 100), 1)
            
            # Si des mains sont détectées
            if results.multi_hand_landmarks:
                for hand_landmarks, handedness in zip(results.multi_hand_landmarks, 
                                                    results.multi_handedness):
                    # Déterminer si main gauche ou droite
                    hand_label = handedness.classification[0].label
                    
                    # Dessiner les informations de la main
                    x, y = draw_hand_info(image, hand_landmarks, hand_label)
                    
                    # Afficher des informations supplémentaires en console
                    print(f"Main {hand_label}: X={x:.2f}, Y={y:.2f}")
            
            else:
                # Aucune main détectée
                cv2.putText(image, "AUCUNE MAIN DETECTEE", (w//2-100, h//2), 
                           cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 0, 255), 2)
            
            # Afficher le FPS
            cv2.putText(image, "Tracker Main - MediaPipe", (10, h-10), 
                       cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 1)
            
            # Afficher l'image
            cv2.imshow('Hand Tracking - MediaPipe', image)
            
            # Quitter avec 'q'
            if cv2.waitKey(1) & 0xFF == ord('q'):
                break
        
        # Libérer les ressources
        cap.release()
        cv2.destroyAllWindows()

if __name__ == "__main__":
    main()