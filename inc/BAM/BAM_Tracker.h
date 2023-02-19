#pragma once
#ifdef ENABLE_BAM
/**
*  Copyright (C) 2016 Rafal Janicki
*
*  This software is provided 'as-is', without any express or implied
*  warranty.  In no event will the authors be held liable for any damages
*  arising from the use of this software.
*
*  Permission is granted to anyone to use this software for any purpose,
*  including commercial applications, and to alter it and redistribute it
*  freely, subject to the following restrictions:
*
*  1. The origin of this software must not be misrepresented; you must not
*     claim that you wrote the original software. If you use this software
*     in a product, an acknowledgment in the product documentation would be
*     appreciated but is not required.
*  2. Altered source versions must be plainly marked as such, and must not be
*     misrepresented as being the original software.
*  3. This notice may not be removed or altered from any source distribution.
*
*  Rafal Janicki
*  ravarcade@gmail.com
*/

// #include <windows.h> // required

/**
 *
 * Right Hand coord system. (0,0,0) is center of screen surface.
 *
 * Axis:
 *  +X - to right,
 *  +Y - to top of screen (in cab it will be away from viewer)
 *  +Z - perpedicular to screen surface.
 *
 * Units: millimeters
 *
 */

namespace BAM_Tracker {

	/// <summary>
	/// The shared memory / memory-mapped file name
	/// </summary>
	static const char *SharedMemoryFileName = "BAM-Tracker-Shared-Memory";

	/// <summary>
	/// Single captured data about player position.
	/// </summary>
	struct TPlayerData {
		double StartPosition[4]; // x,y,z [mm] + timestamp [ms]
		double EndPosition[4];   // x,y,z [mm] + timestamp [ms]
		double EyeVec[3]; // [normalized vector]
		int FrameCounter;
	};

	/// <summary>
	/// Data recived from BAM Tracker.
	/// </summary>
	struct TData {
		// values for HRTimer to synchonize tracker and client timers.
		LARGE_INTEGER Time_StartValue;
		double Time_OneMillisecond;

		// Size of screen in millimeters.
		double ScreenWidth, ScreenHeight; 

		// Head Tracking Data is double buffered
		TPlayerData Data[2];
		int UsedDataSlot; // info in what buffer is last head tracking data
	};


	/// <summary>
	/// All communication with BAM-Tracker
	/// </summary>
	class BAM_Tracker_Client {
	private:
		HANDLE m_hMapFile; /// memory-mapped file handle.
		TData *m_pData;    /// pointer to data from BAM Tracker
		double m_Time_OneMillisecond;
		LARGE_INTEGER m_Time_StartValue;

		/// <summary>
		/// Interpolates the position.
		/// </summary>
		/// <param name="P">Output position [3 x double].</param>
		/// <param name="d">Played data: start and end position with timestamps.</param>
		/// <param name="Tnow">Current time.</param>
		void InterpolatePosition(double *P, TPlayerData &d, double Tnow)
		{
			// 'a' - Blending factor between two points. 
			// Based on time stamps for start & end point (Tstart & Tend) and current time (Tnow).
			double Tstart = d.StartPosition[3];
			double Tend = d.EndPosition[3];
			double Tepsilon = 1.0; // 1 ms differenc is to small but it will never happed.
			double dT = Tend - Tstart;
			
			// Block interpolation if for long time there is no new HT data
			constexpr double dTmin = 200; // time, when we start to slow down move if there is no new HT data
			constexpr double dTmax = 400; // max time without HT data

			double t = Tnow - Tend;
			double kt = 1.0;
			if (t > dTmin) 
			{
				if (t < dTmax)
				{
					kt = (dTmax - t) / (dTmax - dTmin);
				}
				else
				{
					kt = 0.0;
				}
			}

			double dN = kt * t +Tend - Tstart;
			double a = (dT > Tepsilon) ? dN / dT : 1.0;

			// calc interpolated position:
			for (int i = 0; i < 3; ++i)
			{
				P[i] = a*d.EndPosition[i] + (1.0 - a)*d.StartPosition[i];
			}
		}

	public:
		/// <summary>
		/// Initializes a new instance of the <see cref="BAM_Tracker_Client"/> class.
		/// Initialize BAM-Tracker memory-mapped file (shared memory).
		/// </summary>
		BAM_Tracker_Client()
		{
			// init timer in case, there is no BAM-Tracker
			LARGE_INTEGER proc_freq;
			if (!QueryPerformanceFrequency(&proc_freq))
				throw TEXT("QueryPerformanceFrequency() failed");

			m_Time_OneMillisecond = (double)proc_freq.QuadPart;
			m_Time_OneMillisecond = 1000.0 / m_Time_OneMillisecond;
			QueryPerformanceCounter(&m_Time_StartValue);

			// get BAM-Tracker memory
			m_pData = NULL;

			m_hMapFile = OpenFileMappingA(
				FILE_MAP_READ,			// read access
				FALSE,					// do not inherit the name
				SharedMemoryFileName);	// "filename" with BAM-Tracker shared memory

			if (m_hMapFile) {
				m_pData = (TData *)MapViewOfFile(m_hMapFile, FILE_MAP_READ, 0, 0, sizeof(TData));
				if (m_pData == NULL) {
					CloseHandle(m_hMapFile);
					m_hMapFile = NULL;
				}
				else
				{
					m_Time_OneMillisecond = m_pData->Time_OneMillisecond;
					m_Time_StartValue = m_pData->Time_StartValue;
				}
			}
		}

		/// <summary>
		/// Finalizes an instance of the <see cref="BAM_Tracker_Client"/> class.
		/// Releases resource: memory-mapped file;
		/// </summary>
		~BAM_Tracker_Client()
		{
			if (m_pData) {
				UnmapViewOfFile(m_pData);
			}

			if (m_hMapFile) {
				CloseHandle(m_hMapFile);
			}
		}

		/// <summary>
		/// Get current time (synchronized with BAM Tracker if BT is present).
		/// </summary>
		/// <returns>Current time in milliseconds.</returns>
		double GetTime(void)
		{
			LARGE_INTEGER stop;
			QueryPerformanceCounter(&stop);
			return ((stop.QuadPart - m_Time_StartValue.QuadPart) * m_Time_OneMillisecond);
		}

		/// <summary>
		/// Determines whether [is bam tracker present].
		/// </summary>
		/// <returns></returns>
		bool IsBAMTrackerPresent()
		{
			return m_pData != NULL;
		}

		/// <summary>
		/// Gets the width of the screen.
		/// </summary>
		/// <returns></returns>
		double GetScreenWidth() { return m_pData ? m_pData->ScreenWidth : 0; }
		double GetScreenHeight() { return m_pData ? m_pData->ScreenHeight : 0; }

		bool GetPosition(double &X, double &Y, double &Z)
		{
			if (!m_pData)
				return false;

			static double lastP[4] = { 0 };
			static TPlayerData lastPD = { 0 };
			static TPlayerData d2 = { 0 };
			
			TPlayerData d = m_pData->Data[m_pData->UsedDataSlot];

			double Tnow = GetTime(); // get current time

			bool isNewEndPoint = abs(lastPD.EndPosition[3] - d.EndPosition[3]) > 0.1;
			if (isNewEndPoint) { 
				// if we have new target point, use last position and new target point as line for interpolation
				d2.StartPosition[0] = lastP[0];
				d2.StartPosition[1] = lastP[1];
				d2.StartPosition[2] = lastP[2];
				d2.StartPosition[3] = lastP[3];
				d2.EndPosition[0] = d.EndPosition[0];
				d2.EndPosition[1] = d.EndPosition[1];
				d2.EndPosition[2] = d.EndPosition[2];
				d2.EndPosition[3] = d.EndPosition[3];
			}

			double P[3], P2[3];
			InterpolatePosition(P, d, Tnow);
			if (abs(d.EndPosition[3] - d.StartPosition[3]) > 5.0)
			{
				InterpolatePosition(P2, d2, Tnow);
				double a = 0.8 * (Tnow - d2.StartPosition[3]) / (d.EndPosition[3] - d.StartPosition[3]);
				if (a < 0.0) a = 0.0;
				if (a > 1.0) a = 1.0;
				for (int i = 0; i < 3; ++i)
				{
					P[i] = a * P[i] + (1 - a)*P2[i];
				}
			}

			X = lastP[0] = P[0];
			Y = lastP[1] = P[1];
			Z = lastP[2] = P[2];
			lastP[3] = Tnow;
			if (isNewEndPoint)
			{
				lastPD = d;
			}

			return true;
		}

		/// <summary>
		/// Gets the eyes position. (Call it instead GetViewerPosition for stereo 3D).
		/// Common InterPupillar Distance = 65 mm. See: https://en.wikipedia.org/wiki/Pupillary_distance
		/// </summary>
		/// <param name="LeftEye">The left eye.</param>
		/// <param name="RightEye">The right eye.</param>
		/// <param name="IDP">The idp.</param>
		/// <returns></returns>
		bool GetEyesPosition(double LeftEye[3], double RightEye[3], double IDP = 65.0)
		{
			double P[3];
			if (!GetPosition(P[0], P[1], P[2])) {
				return false;
			}

			IDP *= 0.5;
			double *EyeVec = m_pData->Data[m_pData->UsedDataSlot].EyeVec;
			for (int i = 0; i < 3; ++i)
			{
				LeftEye[i] = P[i] - IDP * EyeVec[i];
				RightEye[i] = P[i] + IDP * EyeVec[i];
			}
			return true;
		}
	};

}
#endif