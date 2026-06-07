using UnityEngine;

namespace MalumMenu;

public class TextField
{
    private string _content = "";
    private bool _focused = false;
    private float _lastBlinkTime = 0f;
    private bool _cursorVisible = true;
    private Rect _fieldRect = Rect.zero;
    private float _cursorBlinkTime = 0.5f;

    public bool IsFocused => _focused;
    public string Content
    {
        get => _content;
        set => _content = value;
    }

    public TextField(string initialContent = "")
    {
        _content = initialContent;
    }

    public void Draw(int width = 200, int height = 20)
    {
        GUILayout.Box("", GUILayout.Width(width), GUILayout.Height(height));

        if (Event.current.type == EventType.Repaint)
        {
            _fieldRect = GUILayoutUtility.GetLastRect();
        }

        // Handle mouse click to focus
        if (Event.current.type == EventType.MouseDown)
        {
            if (_fieldRect.Contains(Event.current.mousePosition))
            {
                _focused = true;
                _lastBlinkTime = Time.time;
                _cursorVisible = true;
                Event.current.Use();
            }
            else
            {
                _focused = false;
            }
        }

        // Handle keyboard input
        if (_focused && Event.current.type == EventType.KeyDown)
        {
            if (Event.current.keyCode == KeyCode.Backspace)
            {
                if (_content.Length > 0)
                {
                    _content = _content.Substring(0, _content.Length - 1);
                    Event.current.Use();
                }
            }
            else if (Event.current.character != '\0' && !char.IsControl(Event.current.character))
            {
                _content += Event.current.character;
                Event.current.Use();
            }
        }

        // Display text content
        GUI.Label(new Rect(_fieldRect.x + 5, _fieldRect.y + 2, _fieldRect.width - 10, _fieldRect.height), _content);

        // Handle cursor blinking
        if (_focused)
        {
            if (Time.time - _lastBlinkTime > _cursorBlinkTime)
            {
                _cursorVisible = !_cursorVisible;
                _lastBlinkTime = Time.time;
            }

            // Draw blinking cursor
            if (_cursorVisible)
            {
                Vector2 textSize = GUI.skin.label.CalcSize(new GUIContent(_content));
                GUI.Label(new Rect(_fieldRect.x + textSize.x + 7, _fieldRect.y + 2, 10, _fieldRect.height - 4), "|");
            }
        }
    }

    public void Unfocus()
    {
        _focused = false;
    }
}
