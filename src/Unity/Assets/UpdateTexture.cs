using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System;
using System.Runtime.InteropServices;

public class UpdateTexture : MonoBehaviour
{
    public Material cubeMat;
    int i = 0;
	bool showWebCamera = false;

	[Range(0.0f, 4.0f)]
	public float amount = 1.0f;

	int m_resolutionWidth = 1246;
	int m_resolutionHeight = 800;
	int m_frameRate = 1000;
	int m_shutterSpeedFps = 2000;


	[DllImport("SimplePUCLIB")]
	private static extern void ReadInfincamImage(ref Color32[] rawImage, int width, int height);

	[DllImport("SimplePUCLIB")]
	private static extern void SetupInfincam();

	[DllImport("SimplePUCLIB")]
	private static extern void SetInfincamResolution(int width, int height);

	[DllImport("SimplePUCLIB")]
	private static extern void SetInfincamFramerateShutter(int nFramerate, int nShutterSpeedFps);

	[DllImport("SimplePUCLIB")]
	private static extern int OpenInfincam(int deviceID);

	[DllImport("SimplePUCLIB")]
	private static extern bool IsInfincamOpen();

	[DllImport("SimplePUCLIB")]
	public static extern bool TeardownInfincam();

	[DllImport("SimplePUCLIB")]
	private static extern void FilterWebCamImage(ref Color32[] inputImage, ref Color32[] outputImage, int width, int height, float amount);



	private WebCamTexture _webcam;
	private Texture2D _cameraTexture;
	private Color32[] outputImage;

	void Start()
    {
		SetupInfincam();
		SetInfincamResolution(m_resolutionWidth, m_resolutionHeight);
		SetInfincamFramerateShutter(m_frameRate, m_shutterSpeedFps);
		OpenInfincam(0);
		if (IsInfincamOpen())
		{
			outputImage = new Color32[m_resolutionWidth * m_resolutionHeight];
			_cameraTexture = new Texture2D(m_resolutionWidth, m_resolutionHeight);
			cubeMat.mainTexture = _cameraTexture;
			showWebCamera = false;
		}
		else
        {
			StartWebCamera();
			showWebCamera = true;
		}

	}
	void Update()
    {
		if (showWebCamera)
		{
			UpdateWebCamera();
			return;
		}

		var rawImage = outputImage;
		ReadInfincamImage(ref rawImage, m_resolutionWidth, m_resolutionHeight);
		_cameraTexture.SetPixels32(rawImage);
		_cameraTexture.Apply();
	}

	void OnApplicationQuit()
	{
		TeardownInfincam();
	}


	void StartWebCamera()
    {
		_webcam = new WebCamTexture();
		_webcam.Play();
		_cameraTexture = new Texture2D(_webcam.width, _webcam.height);
		cubeMat.mainTexture = _cameraTexture;
		outputImage = new Color32[_webcam.width * _webcam.height];
	}

	void UpdateWebCamera()
    {
		if (_webcam.isPlaying)
		{
			var inputRawImage = _webcam.GetPixels32();
			var outputRawImage = outputImage;
			FilterWebCamImage(ref inputRawImage, ref outputRawImage, _webcam.width, _webcam.height, amount);
			_cameraTexture.SetPixels32(outputRawImage);
			_cameraTexture.Apply();
		}
	}

}
