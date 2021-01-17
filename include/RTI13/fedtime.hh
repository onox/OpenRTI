// HLA 1.3 Header "fedtime.hh"

#ifndef HLA_FEDTIME_HH
#define HLA_FEDTIME_HH

#include "RTI.hh"
#include <iosfwd>

class FEDTIME_EXPORT RTIfedTime : public RTI::FedTime {
 public:
  RTIfedTime();
  RTIfedTime(const double&);
  RTIfedTime(const RTI::FedTime&);
  RTIfedTime(const RTIfedTime&);
  virtual ~RTIfedTime();

  virtual void setZero();
  virtual RTI::Boolean isZero();
  virtual void setEpsilon();
  virtual void setPositiveInfinity();
  virtual RTI::Boolean isPositiveInfinity();

  virtual RTI::FedTime& operator+=(const RTI::FedTime&)
    RTI_THROW ((RTI::InvalidFederationTime));

  virtual RTI::FedTime& operator-=(const RTI::FedTime&)
    RTI_THROW ((RTI::InvalidFederationTime));

  virtual RTI::Boolean operator<=(const RTI::FedTime&) const
    RTI_THROW ((RTI::InvalidFederationTime));

  virtual RTI::Boolean operator<(const RTI::FedTime&) const
    RTI_THROW ((RTI::InvalidFederationTime));

  virtual RTI::Boolean operator>=(const RTI::FedTime&) const
    RTI_THROW ((RTI::InvalidFederationTime));

  virtual RTI::Boolean operator>(const RTI::FedTime&) const
    RTI_THROW ((RTI::InvalidFederationTime));

  virtual RTI::Boolean operator==(const RTI::FedTime&) const
    RTI_THROW ((RTI::InvalidFederationTime));

  virtual RTI::FedTime& operator=(const RTI::FedTime&)
    RTI_THROW ((RTI::InvalidFederationTime));

  virtual int encodedLength() const;
  virtual void encode(char *) const;
  virtual int getPrintableLength() const;
  virtual void getPrintableString(char *);

  bool isEpsilon() const;
  double getTime() const;

  RTI::Boolean operator==(const double&) const
    RTI_THROW ((RTI::InvalidFederationTime));

  RTI::Boolean operator!=(const RTI::FedTime&) const
    RTI_THROW ((RTI::InvalidFederationTime));

  RTI::Boolean operator!=(const double&) const
    RTI_THROW ((RTI::InvalidFederationTime));

  RTI::Boolean operator<=(const double&) const
    RTI_THROW ((RTI::InvalidFederationTime));

  RTI::Boolean operator<(const double&) const
    RTI_THROW ((RTI::InvalidFederationTime));

  RTI::Boolean operator>=(const double&) const
    RTI_THROW ((RTI::InvalidFederationTime));

  RTI::Boolean operator>(const double&) const
    RTI_THROW ((RTI::InvalidFederationTime));

  RTI::FedTime& operator=(const RTIfedTime&)
    RTI_THROW ((RTI::InvalidFederationTime));

  RTI::FedTime& operator=(const double&)
    RTI_THROW ((RTI::InvalidFederationTime));

  RTI::FedTime& operator*=(const RTI::FedTime&)
    RTI_THROW ((RTI::InvalidFederationTime));

  RTI::FedTime& operator/=(const RTI::FedTime&)
    RTI_THROW ((RTI::InvalidFederationTime));

  RTI::FedTime& operator+=(const double&)
    RTI_THROW ((RTI::InvalidFederationTime));

  RTI::FedTime& operator-=(const double&)
    RTI_THROW ((RTI::InvalidFederationTime));

  RTI::FedTime& operator*=(const double&)
    RTI_THROW ((RTI::InvalidFederationTime));

  RTI::FedTime& operator/=(const double&)
    RTI_THROW ((RTI::InvalidFederationTime));

  RTIfedTime operator+(const RTI::FedTime&) const
    RTI_THROW ((RTI::InvalidFederationTime));

  RTIfedTime operator+(const double&) const
    RTI_THROW ((RTI::InvalidFederationTime));

  RTIfedTime operator-(const RTI::FedTime&) const
    RTI_THROW ((RTI::InvalidFederationTime));

  RTIfedTime operator-(const double&) const
    RTI_THROW ((RTI::InvalidFederationTime));

  RTIfedTime operator*(const RTI::FedTime&) const
    RTI_THROW ((RTI::InvalidFederationTime));

  RTIfedTime operator*(const double&) const
    RTI_THROW ((RTI::InvalidFederationTime));

  RTIfedTime operator/(const RTI::FedTime&) const
    RTI_THROW ((RTI::InvalidFederationTime));

  RTIfedTime operator/(const double&) const
    RTI_THROW ((RTI::InvalidFederationTime));

private:
  double _fedTime;
};

FEDTIME_EXPORT RTIfedTime operator+(const double&, const RTI::FedTime&);
FEDTIME_EXPORT RTIfedTime operator-(const double&, const RTI::FedTime&);
FEDTIME_EXPORT RTIfedTime operator*(const double&, const RTI::FedTime&);
FEDTIME_EXPORT RTIfedTime operator/(const double&, const RTI::FedTime&);

FEDTIME_EXPORT std::ostream& operator<<(std::ostream&, const RTI::FedTime&);

#endif // HLA_FEDTIME_HH
