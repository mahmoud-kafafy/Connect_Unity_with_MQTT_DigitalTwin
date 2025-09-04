using System.Collections.Generic;
using UnityEngine;

public class ConveyorBelt : MonoBehaviour
{
    public float speed = 2f;
    public Vector3 direction = Vector3.left;
    private bool isRunning = true;

    // NEW: keep track of who’s touching the belt
    private readonly HashSet<Rigidbody> touching = new HashSet<Rigidbody>();

    private void OnCollisionEnter(Collision collision)
    {
        if (collision.rigidbody != null)
            touching.Add(collision.rigidbody);
    }

    private void OnCollisionExit(Collision collision)
    {
        if (collision.rigidbody != null)
            touching.Remove(collision.rigidbody);
    }

    private void OnCollisionStay(Collision collision)
    {
        Rigidbody rb = collision.rigidbody;
        if (rb != null && isRunning)
        {
            rb.WakeUp(); // keep awake while running
            rb.MovePosition(rb.position + direction.normalized * speed * Time.deltaTime);
        }
    }

    public void MoveForward()
    {
        Debug.Log("Forward button pressed!");
        direction = Vector3.left;  // +X in your setup
        isRunning = true;
        WakeAndNudge();
    }

    public void MoveBackward()
    {
        Debug.Log("Backward button pressed!");
        direction = Vector3.right; // -X
        isRunning = true;
        WakeAndNudge();
    }

    public void Stop()
    {
        Debug.Log("Stop button pressed!");
        isRunning = false;
    }

    // NEW: wake any bodies already resting on the belt and give a tiny nudge
    private void WakeAndNudge()
    {
        Vector3 tiny = direction.normalized * 0.001f; // tiny push to re-trigger contacts
        foreach (var rb in touching)
        {
            if (rb == null) continue;
            rb.WakeUp();
            rb.MovePosition(rb.position + tiny);
        }
    }
}
