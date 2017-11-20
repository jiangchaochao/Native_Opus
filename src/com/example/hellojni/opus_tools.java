public class opus_tools{

    static{
        System.loadLibrary("OpusTools");
    }

    public native int OpusCreate(int Fs, int channels, int application);
    public native int OpusEncode(short pcm[], int analysis_frame_size, char data[], int max_data_bytes);
}
