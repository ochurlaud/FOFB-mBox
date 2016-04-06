#ifndef CORRECTIONPROCESSOR_H
#define CORRECTIONPROCESSOR_H

#include <armadillo>

class ADC;
class RFM;

class CorrectionProcessor
{
public:
    /**
     * @brief Constructor
     */
    explicit CorrectionProcessor();

    /**
     * @brief Calculate the correction to apply.
     *
     * @param[in] diffX BPM values for the x axis
     * @param[in] diffY BPM values for the y axis
     * @param[in] newInjection Was there a new injection?
     * @param[out] Data_CMx Corrector values for the x axis
     * @param[out] Data_CMy Corrector values for the y axis
     * @param[in] type int value in the following set:
     *                  * Correction::None (= `0b00`)
     *                  * Correction::Horizontal (= `0b01`)
     *                  * Correction::Vertical (= `0b10`)
     *                  * Correction::All (= `0b11`)
     */
    int correct(const arma::vec& diffX, const arma::vec& diffY,
                const bool newInjection,
                arma::vec& Data_CMx, arma::vec& Data_CMy,
                const int type );

    /**
     * @brief Check if the correction is correct.
     *
     * This function should not be called in `--rw` or `--experiment` mode, as
     * it's time consuming.
     *
     * @note This is an empty function: not implemented for now.
     *
     * @return  ???
     */
    int checkCorrection();

    /**
     * @brief Set the PID parameters.
     */
    void setPID(double P, double I, double D) { m_P = P; m_I = I; m_D = D;};

    /**
     * @brief Set the correctors.
     */
    void setCMs(arma::vec CMx, arma::vec CMy);

    /**
     * @brief Initialize the injection count.
     *
     *  * count start = frequency/1000
     *  * count stop  = frequency*60/1000
     */
    void setInjectionCnt(double frequency);

    /**
     * @brief Function that call calcSmat() for both x and y axes.
     *
     * @param SmatX, SmatY Matrices to inverse (both axes)
     * @param IvecX, IvecY ????
     * @param CMWeight True if the correction should be weighted or not.
     */
    void setSmat(arma::mat &SmatX, arma::mat &SmatY, double IvecX, double IvecY, bool weightedCorr);

    int m_rmsErrorCnt;
private:

    /**
     * @brief Calculate the inverse of the S matrix.
     *
     * @param[in] Smat Matrix to inverse
     * @param[in] Ivec ????
     * @param[out] CMWeight Vector of weight to apply for each corrector
     * @param[out] SmatInv Inversed matrix
     */
    void calcSmat(const arma::mat &Smat, double Ivec, arma::vec &CMWeight, arma::mat &SmatInv);

    int m_injectionCnt;
    int m_injectionStopCnt;
    int m_injectionStartCnt;

    bool m_useCMWeight;
    arma::vec m_CMWeightX, m_CMWeightY;

    double m_P, m_I, m_D, m_currentP;
    double m_lastrmsX, m_lastrmsY;

    arma::mat m_SmatInvX, m_SmatInvY;
    arma::vec m_CMx, m_CMy;
    arma::vec m_dCORxPID, m_dCORyPID;
    arma::vec m_dCORlastX, m_dCORlastY;
    arma::vec m_Xsum, m_Ysum;
    arma::vec m_Data_CMx, m_Data_CMy;
};

#endif // CORRECTIONPROCESSOR_H
