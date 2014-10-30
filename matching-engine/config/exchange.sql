-- phpMyAdmin SQL Dump
-- version 4.1.14
-- http://www.phpmyadmin.net
--
-- Host: localhost
-- Generation Time: Oct 30, 2014 at 04:25 PM
-- Server version: 5.5.39-MariaDB
-- PHP Version: 5.5.18

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;

--
-- Database: `exchange`
--

-- --------------------------------------------------------

--
-- Table structure for table `configuration`
--

CREATE TABLE IF NOT EXISTS `configuration` (
  `Id` int(11) NOT NULL AUTO_INCREMENT,
  `Name` varchar(64) NOT NULL,
  `Value` varchar(64) NOT NULL,
  PRIMARY KEY (`Id`)
) ENGINE=InnoDB  DEFAULT CHARSET=latin1 AUTO_INCREMENT=6 ;

--
-- Dumping data for table `configuration`
--

INSERT INTO `configuration` (`Id`, `Name`, `Value`) VALUES
(1, 'engine.start_time', '08:55:00.000'),
(2, 'engine.stop_time', '23:55:00.000'),
(3, 'engine.opening_auction_duration', '3'),
(4, 'engine.closing_auction_duration', '3'),
(5, 'engine.intraday_auction_duration', '3');

-- --------------------------------------------------------

--
-- Table structure for table `instruments`
--

CREATE TABLE IF NOT EXISTS `instruments` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(20) NOT NULL,
  `isin` varchar(12) NOT NULL,
  `securitycode` int(11) NOT NULL,
  `type` int(11) NOT NULL,
  `currency` varchar(3) NOT NULL,
  `close` int(11) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB  DEFAULT CHARSET=latin1 AUTO_INCREMENT=7 ;

--
-- Dumping data for table `instruments`
--

INSERT INTO `instruments` (`id`, `name`, `isin`, `securitycode`, `type`, `currency`, `close`) VALUES
(1, 'MICHELIN', 'FR0000121261', 1, 1, 'EUR', 1252),
(2, 'NATIXIS', 'FR0000120685', 2, 1, 'EUR', 392),
(3, 'ALSTOM', 'FR0010220475', 3, 1, 'EUR', 2555),
(4, 'SAFRAN', 'FR0000073272', 4, 1, 'EUR', 4448),
(5, 'AREVA', 'FR0011027143', 5, 1, 'EUR', 1423),
(6, 'AXA', 'FR0000120628', 6, 1, 'EUR', 1882);

-- --------------------------------------------------------

--
-- Table structure for table `users`
--

CREATE TABLE IF NOT EXISTS `users` (
  `Id` int(11) NOT NULL AUTO_INCREMENT,
  `username` varchar(10) NOT NULL,
  `password` varchar(64) NOT NULL,
  `enable` tinyint(1) NOT NULL,
  `endofvalidity` datetime NOT NULL,
  PRIMARY KEY (`Id`)
) ENGINE=InnoDB  DEFAULT CHARSET=latin1 AUTO_INCREMENT=4 ;

--
-- Dumping data for table `users`
--

INSERT INTO `users` (`Id`, `username`, `password`, `enable`, `endofvalidity`) VALUES
(1, 'SGCIB', 'c40180995b9b6146e16c5ec73b236ae7a481f3c91ce5e9f9f3eb37f6f21f07c9', 1, '2014-06-24 00:00:00'),
(2, 'CACIB', '28c4d25bada78027ac04f9d3dc041e29099bcb558d8254ae0b9584bceefc3093', 0, '2014-03-13 00:00:00'),
(3, 'BNPCIB', '0c3c89248b33b082ab9caf589ed018419e3b17c266e60b3279a5c92f702c1bb2', 1, '2014-05-14 00:00:00');

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
