using System;
using System.Collections.Concurrent;
using UnityEngine;

public class UnityMainThreadDispatcher : MonoBehaviour
{
    private static readonly ConcurrentQueue<Action> _actions = new ConcurrentQueue<Action>();
    private static UnityMainThreadDispatcher _instance;

    void Awake()
    {
        _instance = this;
    }

    public static void Enqueue(Action action)
    {
        if (_instance == null)
        {
            Debug.LogError("UnityMainThreadDispatcher not present in scene!");
            return;
        }
        _actions.Enqueue(action);
    }

    void Update()
    {
        while (_actions.TryDequeue(out var action))
        {
            action?.Invoke();
        }
    }
}
