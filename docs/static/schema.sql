/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `child_botserv_bots`
--

DROP TABLE IF EXISTS `child_botserv_bots`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `child_botserv_bots` (
  `name` varchar(50) NOT NULL DEFAULT '',
  `ident` varchar(50) NOT NULL DEFAULT '',
  `host` varchar(50) NOT NULL DEFAULT ''
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `child_botserv_chans`
--

DROP TABLE IF EXISTS `child_botserv_chans`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `child_botserv_chans` (
  `chan` varchar(50) NOT NULL DEFAULT '',
  `bot` varchar(50) NOT NULL DEFAULT ''
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `child_chan_access`
--

DROP TABLE IF EXISTS `child_chan_access`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `child_chan_access` (
  `chname` varchar(50) NOT NULL DEFAULT '',
  `username` varchar(255) NOT NULL DEFAULT '',
  `level` int(11) NOT NULL DEFAULT '0',
  `automode` int(11) NOT NULL DEFAULT '1',
  `suspended` int(11) NOT NULL,
  `uflags` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `child_chans`
--

DROP TABLE IF EXISTS `child_chans`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `child_chans` (
  `chname` varchar(50) DEFAULT NULL,
  `founder` varchar(50) DEFAULT NULL,
  `entrymsg` blob,
  `options` int(11) DEFAULT NULL,
  `mlock` varchar(32) NOT NULL DEFAULT '',
  `autolimit` int(11) NOT NULL DEFAULT '0',
  `lastseen` int(11) NOT NULL DEFAULT '0',
  `regtime` int(11) NOT NULL,
  `topic` blob NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `child_links`
--

DROP TABLE IF EXISTS `child_links`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `child_links` (
  `master` varchar(50) DEFAULT NULL,
  `slave` varchar(50) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `child_trusts`
--

DROP TABLE IF EXISTS `child_trusts`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `child_trusts` (
  `hostname` varchar(255) DEFAULT NULL,
  `clones` int(11) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `child_users`
--

DROP TABLE IF EXISTS `child_users`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `child_users` (
  `username` varchar(50) DEFAULT NULL,
  `authlevel` int(11) DEFAULT NULL,
  `seen` int(11) DEFAULT NULL,
  `vhost` varchar(200) DEFAULT NULL,
  `md5_pass` varchar(32) NOT NULL DEFAULT '',
  `pwhash` varchar(128) NOT NULL DEFAULT '',  /* Size is libsodium's crypto_pwhash_STRBYTES */
  `options` int(11) NOT NULL DEFAULT '0',
  `timeout` int(11) NOT NULL DEFAULT '0',
  `email` varchar(100) NOT NULL DEFAULT '',
  `regtime` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;
